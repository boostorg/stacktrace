// Copyright Antony Polukhin, 2023-2024.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stacktrace/safe_dump_to.hpp>

#include <boost/assert.hpp>

#include <cstddef>
#include <dlfcn.h>

#if defined(__x86_64__) || defined(_M_X64)
#define BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING 1
#else
#define BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING 0
#endif

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
#include <mutex>
#include <unordered_map>
#endif

namespace {

constexpr std::size_t kStacktraceDumpSize = 4096;

// Developer note: helper to experiment with layouts of different
// exception headers https://godbolt.org/z/z7jdd7Tfx

inline void* get_current_exception_raw_ptr() noexcept {
  // https://github.com/gcc-mirror/gcc/blob/16e2427f50c208dfe07d07f18009969502c25dc8/libstdc%2B%2B-v3/libsupc%2B%2B/eh_ptr.cc#L147
  auto exc_ptr = std::current_exception();
  return *static_cast<void**>(static_cast<void*>(&exc_ptr));
}

// https://github.com/llvm/llvm-project/blob/b3dd14ce07f2750ae1068fe62abbf2f3bd2cade8/libcxxabi/src/cxa_exception.h
struct cxa_exception_begin_llvm {
    const char* reserve;
    size_t referenceCount;
};

cxa_exception_begin_llvm* exception_begin_llvm_ptr(void* ptr) noexcept {
  constexpr std::size_t kExceptionBeginOffset = (
    sizeof(void*) == 8 ? 128 : 80
  );
  return reinterpret_cast<cxa_exception_begin_llvm*>(
    static_cast<char*>(ptr) - kExceptionBeginOffset
  );
}

// https://github.com/gcc-mirror/gcc/blob/5d2a360f0a541646abb11efdbabc33c6a04de7ee/libstdc%2B%2B-v3/libsupc%2B%2B/unwind-cxx.h#L100
struct cxa_exception_begin_gcc {
    size_t referenceCount;
    const char* reserve;
};

cxa_exception_begin_gcc* exception_begin_gcc_ptr(void* ptr) noexcept {
  constexpr std::size_t kExceptionBeginOffset = (
    sizeof(void*) == 8 ? 128 : 96
  );
  return reinterpret_cast<cxa_exception_begin_gcc*>(
    static_cast<char*>(ptr) - kExceptionBeginOffset
  );
}

struct decrement_on_destroy {
  std::size_t& to_decrement;

  ~decrement_on_destroy() { --to_decrement; }
};

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
// Inspired by the coursework by Andrei Nekrashevich in the `libsfe`
std::mutex mutex;
std::unordered_map<void*, const char*> exception_to_dump_mapping;
#endif

}  // namespace

namespace __cxxabiv1 {

#if defined(__GNUC__) && defined(__ELF__)

// libc++-runtime specific function
extern "C" BOOST_NOINLINE BOOST_SYMBOL_VISIBLE __attribute__((weak))
void __cxa_increment_exception_refcount(void *primary_exception) throw();

static bool is_libcpp_runtime() noexcept {
    return __cxa_increment_exception_refcount;
}

#else

static bool is_libcpp_runtime() noexcept { return false; }

#endif

static const char*& reference_to_empty_padding(void* ptr) noexcept {
  if (is_libcpp_runtime()) {
    // libc++-runtime
    BOOST_ASSERT_MSG(
      sizeof(void*) == 4,
      "32bit platforms are unsupported with libc++ runtime padding reusage. "
      "Please report this issue to the library maintainters."
    );
    return exception_begin_llvm_ptr(ptr)->reserve;
  }

  return exception_begin_gcc_ptr(ptr)->reserve;
}

extern "C" BOOST_SYMBOL_EXPORT
void* __cxa_allocate_exception(size_t thrown_size) throw() {
  static const auto orig_allocate_exception = []() {
    void* const ptr = ::dlsym(RTLD_NEXT, "__cxa_allocate_exception");
    BOOST_ASSERT_MSG(ptr, "Failed to find '__cxa_allocate_exception'");
    return reinterpret_cast<void*(*)(size_t)>(ptr);
  }();

#ifndef NDEBUG
  static thread_local std::size_t in_allocate_exception = 0;
  BOOST_ASSERT_MSG(in_allocate_exception < 10, "Suspicious recursion");
  ++in_allocate_exception;
  const decrement_on_destroy guard{in_allocate_exception};
#endif

  static constexpr std::size_t kAlign = alignof(std::max_align_t);
  thrown_size = (thrown_size + kAlign - 1) & (~(kAlign - 1));

  void* const ptr = orig_allocate_exception(thrown_size + kStacktraceDumpSize);
  char* const dump_ptr = static_cast<char*>(ptr) + thrown_size;

  constexpr size_t kSkip = 1;
  boost::stacktrace::safe_dump_to(kSkip, dump_ptr, kStacktraceDumpSize);

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
  if (is_libcpp_runtime()) {
    const std::lock_guard<std::mutex> guard{mutex};
    exception_to_dump_mapping[ptr] = dump_ptr;
  } else
#endif
  {
    BOOST_ASSERT_MSG(
      reference_to_empty_padding(ptr) == nullptr,
      "Not zeroed out, unsupported implementation"
    );
    reference_to_empty_padding(ptr) = dump_ptr;
  }

  return ptr;
}

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING

// __cxa_free_exception is not called in libc++ as the
// __cxa_decrement_exception_refcount has an inlined call to
// __cxa_free_exception. Overriding libc++ specific function
extern "C" BOOST_SYMBOL_EXPORT
void __cxa_decrement_exception_refcount(void *thrown_object) throw() {
  BOOST_ASSERT(is_libcpp_runtime());
  if (!thrown_object) {
    return;
  }

  static const auto orig_decrement_refcount = []() {
    void* const ptr = ::dlsym(RTLD_NEXT, "__cxa_decrement_exception_refcount");
    BOOST_ASSERT_MSG(ptr, "Failed to find '__cxa_decrement_exception_refcount'");
    return reinterpret_cast<void(*)(void*)>(ptr);
  }();

  const auto* exception_header = exception_begin_llvm_ptr(thrown_object);

  // The following line has a race and could give false positives and false
  // negatives. In first case we remove the trace earlier, in the second case
  // we get a memory leak.
  if (exception_header->referenceCount == 1) {
    const std::lock_guard<std::mutex> guard{mutex};
    exception_to_dump_mapping.erase(thrown_object);
  }

  orig_decrement_refcount(thrown_object);
}

#endif

}  // namespace __cxxabiv1

namespace boost { namespace stacktrace { namespace impl {

BOOST_SYMBOL_EXPORT const char* current_exception_stacktrace() noexcept {
  void* const exc_raw_ptr = get_current_exception_raw_ptr();
  if (!exc_raw_ptr) {
    return nullptr;
  }

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
  if (__cxxabiv1::is_libcpp_runtime()) {
    const std::lock_guard<std::mutex> guard{mutex};
    const auto it = exception_to_dump_mapping.find(exc_raw_ptr);
    if (it != exception_to_dump_mapping.end()) {
      return it->second;
    } else {
      return nullptr;
    }
  } else
#endif
  {
    return __cxxabiv1::reference_to_empty_padding(exc_raw_ptr);
  }
}

BOOST_SYMBOL_EXPORT void assert_no_pending_traces() noexcept {
#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
  if (__cxxabiv1::is_libcpp_runtime()) {
    const std::lock_guard<std::mutex> guard{mutex};
    BOOST_ASSERT(exception_to_dump_mapping.empty());
  }
#endif
}

}}}  // namespace boost::stacktrace::impl

