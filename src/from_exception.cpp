// Copyright Antony Polukhin, 2023-2024.
// Copyright Andrei Nekrashevich, 2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Inspired by the coursework by Andrei Nekrashevich in the `libsfe`

#define BOOST_STACKTRACE_DYN_LINK
#define BOOST_STACKTRACE_LINK
#include <boost/stacktrace.hpp>

#include <boost/assert.hpp>

#include <cstddef>
#include <exception>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <utility>

#include <dlfcn.h>

namespace {

constexpr std::size_t kStacktraceDumpSize = 4096;

std::mutex mutex;
std::unordered_map<void*, const char*> exception_to_dump;

struct decrement_on_destroy {
  std::size_t& to_decrement;

  ~decrement_on_destroy() { --to_decrement; }
};

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
    exception_to_dump[ptr] = dump_ptr;
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
    exception_to_dump.erase(thrown_object);
  }

  orig_free_exception(thrown_object);
}

struct cxa_exception_begin {
    void *reserve;
    size_t referenceCount;
};

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

  constexpr std::size_t kExceptionBeginOffset = 128;
  const auto* exception_header = reinterpret_cast<const cxa_exception_begin*>(
    static_cast<const char*>(thrown_object) - kExceptionBeginOffset
  );
  if (exception_header->referenceCount == 1) {
    const std::lock_guard<std::mutex> guard{mutex};
    exception_to_dump.erase(thrown_object);
  }

  orig_decrement_refcount(thrown_object);
}

}  // namespace __cxxabiv1

namespace boost { namespace stacktrace {

namespace {

inline void* get_current_exception_raw_ptr() noexcept {
  // https://github.com/gcc-mirror/gcc/blob/16e2427f50c208dfe07d07f18009969502c25dc8/libstdc%2B%2B-v3/libsupc%2B%2B/eh_ptr.cc#L147
  auto exc_ptr = std::current_exception();
  return *static_cast<void**>(static_cast<void*>(&exc_ptr));
}

}  // namespace

BOOST_SYMBOL_EXPORT stacktrace current_exception_stacktrace() {
  void* exc_raw_ptr = get_current_exception_raw_ptr();
  if (!exc_raw_ptr) {
    return stacktrace{0, 0};
  }

  const char* stacktrace_dump_ptr = nullptr;
  {
    const std::lock_guard<std::mutex> guard{mutex};
    auto it = exception_to_dump.find(exc_raw_ptr);
    if (it != exception_to_dump.end()) {
      stacktrace_dump_ptr = it->second;
    }
  }

  if (!stacktrace_dump_ptr) {
    return stacktrace{0, 0};
  }

  return stacktrace::from_dump(stacktrace_dump_ptr, kStacktraceDumpSize);
}

BOOST_SYMBOL_EXPORT std::vector<stacktrace> pending_traces() {
  std::vector<stacktrace> result;

  const std::lock_guard<std::mutex> guard{mutex};
  result.reserve(exception_to_dump.size());
  for (const auto& pair : exception_to_dump) {
    result.push_back(
        stacktrace::from_dump(pair.second, kStacktraceDumpSize)
    );
  }

  return result;
}

}}  // namespace boost::stacktrace

