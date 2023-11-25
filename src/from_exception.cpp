// Copyright Antony Polukhin, 2023-2024.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stacktrace/safe_dump_to.hpp>

#include <boost/assert.hpp>

#include <cstddef>
#include <dlfcn.h>

namespace {

constexpr std::size_t kStacktraceDumpSize = 4096;

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
  constexpr std::size_t kExceptionBeginOffset = 128;
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
  constexpr std::size_t kExceptionBeginOffset = 128;
  return reinterpret_cast<cxa_exception_begin_gcc*>(
    static_cast<char*>(ptr) - kExceptionBeginOffset
  );
}

struct decrement_on_destroy {
  std::size_t& to_decrement;

  ~decrement_on_destroy() { --to_decrement; }
};

}  // namespace

#ifndef BOOST_STACKTRACE_USE_GLOBAL_MAP

namespace __cxxabiv1 {

#if defined(__GNUC__) && defined(__ELF__)

// libc++-runtime specific function
extern "C" BOOST_NOINLINE BOOST_SYMBOL_VISIBLE __attribute__((weak))
void __cxa_decrement_exception_refcount(void *thrown_object) throw();

#endif

static const char*& reference_to_empty_padding(void* ptr) noexcept {
#if defined(__GNUC__) && defined(__ELF__)
  if (__cxa_decrement_exception_refcount) {
      // libc++-runtime
      return exception_begin_llvm_ptr(ptr)->reserve;
  }
#endif
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

  BOOST_ASSERT_MSG(reference_to_empty_padding(ptr) == nullptr,
                   "Not zeroed out, unsupported implementation");
  reference_to_empty_padding(ptr) = dump_ptr;

  return ptr;
}

}  // namespace __cxxabiv1

namespace boost { namespace stacktrace { namespace impl {

BOOST_SYMBOL_EXPORT const char* current_exception_stacktrace() noexcept {
  void* const exc_raw_ptr = get_current_exception_raw_ptr();
  if (!exc_raw_ptr) {
    return nullptr;
  }

  return __cxxabiv1::reference_to_empty_padding(exc_raw_ptr);
}

BOOST_SYMBOL_EXPORT void assert_no_pending_traces() noexcept {}

}}}  // namespace boost::stacktrace::impl

#else

// Inspired by the coursework by Andrei Nekrashevich in the `libsfe`

#include <mutex>
#include <unordered_map>

namespace {

std::mutex mutex;
std::unordered_map<void*, const char*> exception_to_dump_mapping;

}  // namespace

namespace __cxxabiv1 {

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

  {
    const std::lock_guard<std::mutex> guard{mutex};
    exception_to_dump_mapping[ptr] = dump_ptr;
  }

  return ptr;
}

// Not called in libc++ as the __cxa_decrement_exception_refcount has an inlined
// call to __cxa_free_exception
extern "C" BOOST_SYMBOL_EXPORT
void __cxa_free_exception(void* thrown_object) throw() {
  static const auto orig_free_exception = []() {
    void* const ptr = ::dlsym(RTLD_NEXT, "__cxa_free_exception");
    BOOST_ASSERT_MSG(ptr, "Failed to find '__cxa_free_exception'");
    return reinterpret_cast<void(*)(void*)>(ptr);
  }();

#ifndef NDEBUG
  static thread_local std::size_t in_free_exception = 0;
  BOOST_ASSERT_MSG(in_free_exception < 10, "Suspicious recursion");
  ++in_free_exception;
  const decrement_on_destroy guard{in_free_exception};
#endif

  {
    const std::lock_guard<std::mutex> guard{mutex};
    exception_to_dump_mapping.erase(thrown_object);
  }

  orig_free_exception(thrown_object);
}

// libc++ specific function
extern "C" BOOST_SYMBOL_EXPORT
void __cxa_decrement_exception_refcount(void *thrown_object) throw() {
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
  // we get a small memory leak.
  if (exception_header->referenceCount == 1) {
    const std::lock_guard<std::mutex> guard{mutex};
    exception_to_dump_mapping.erase(thrown_object);
  }

  orig_decrement_refcount(thrown_object);
}

}  // namespace __cxxabiv1

namespace boost { namespace stacktrace { namespace impl {

BOOST_SYMBOL_EXPORT const char* current_exception_stacktrace() noexcept {
  void* const exc_raw_ptr = get_current_exception_raw_ptr();
  if (!exc_raw_ptr) {
    return nullptr;
  }

  const std::lock_guard<std::mutex> guard{mutex};
  const auto it = exception_to_dump_mapping.find(exc_raw_ptr);
  if (it != exception_to_dump_mapping.end()) {
    return it->second;
  }

  return nullptr;
}

BOOST_SYMBOL_EXPORT void assert_no_pending_traces() noexcept {
  const std::lock_guard<std::mutex> guard{mutex};
  BOOST_ASSERT(exception_to_dump_mapping.empty());
}

}}}  // namespace boost::stacktrace::impl

#endif

