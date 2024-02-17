// Copyright Antony Polukhin, 2023-2024.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "exception_headers.h"

// At the moment the file is used only on POSIX. _Unwind_Backtrace may be
// available on some platforms only if _GNU_SOURCE is defined.
#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <boost/assert.hpp>
#include <boost/stacktrace/safe_dump_to.hpp>

#include <cstddef>
#include <dlfcn.h>

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
#include <mutex>
#include <unordered_map>

#ifndef BOOST_STACKTRACE_LIBCXX_RUNTIME_MAY_CAUSE_MEMORY_LEAK

#ifdef BOOST_HAS_THREADS

#error On this platform memory leaks are possible if capturing stacktrace from \
        exceptions is enabled and exceptions are thrown concurrently \
        and libc++ runtime is used. \
        \
        Define `BOOST_STACKTRACE_LIBCXX_RUNTIME_MAY_CAUSE_MEMORY_LEAK` to \
        suppress this error if the library would not be used with libc++ \
        runtime (for example, it would be only used with GCC runtime). \
        \
        Otherwise, disable the boost_stacktrace_from_exception library build \
        (for example by `./b2 boost.stacktrace.from_exception=off` option).

#endif

#endif

#endif

namespace {

constexpr std::size_t kStacktraceDumpSize = 4096;

struct decrement_on_destroy {
  std::size_t& to_decrement;

  ~decrement_on_destroy() { --to_decrement; }
};

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
// Inspired by the coursework by Andrei Nekrashevich in the `libsfe`
/*constinit*/ std::mutex g_mapping_mutex;
std::unordered_map<void*, const char*> g_exception_to_dump_mapping;
#endif

}  // namespace

namespace boost { namespace stacktrace { namespace impl {

BOOST_SYMBOL_EXPORT bool& ref_capture_stacktraces_at_throw() noexcept {
  /*constinit*/ thread_local bool g_capture_stacktraces_at_throw{true};
  return g_capture_stacktraces_at_throw;
}

}}}  // namespace boost::stacktrace::impl

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

  if (!boost::stacktrace::impl::ref_capture_stacktraces_at_throw()) {
    return orig_allocate_exception(thrown_size);
  }

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
    const std::lock_guard<std::mutex> guard{g_mapping_mutex};
    g_exception_to_dump_mapping[ptr] = dump_ptr;
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
    const std::lock_guard<std::mutex> guard{g_mapping_mutex};
    g_exception_to_dump_mapping.erase(thrown_object);
  }

  orig_decrement_refcount(thrown_object);
}

#endif

}  // namespace __cxxabiv1

namespace boost { namespace stacktrace { namespace impl {

BOOST_SYMBOL_EXPORT const char* current_exception_stacktrace() noexcept {
  if (!ref_capture_stacktraces_at_throw()) {
    return nullptr;
  }

  auto exc_ptr = std::current_exception();
  void* const exc_raw_ptr = get_current_exception_raw_ptr(&exc_ptr);
  if (!exc_raw_ptr) {
    return nullptr;
  }

#if !BOOST_STACKTRACE_ALWAYS_STORE_IN_PADDING
  if (__cxxabiv1::is_libcpp_runtime()) {
    const std::lock_guard<std::mutex> guard{g_mapping_mutex};
    const auto it = g_exception_to_dump_mapping.find(exc_raw_ptr);
    if (it != g_exception_to_dump_mapping.end()) {
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
    const std::lock_guard<std::mutex> guard{g_mapping_mutex};
    BOOST_ASSERT(g_exception_to_dump_mapping.empty());
  }
#endif
}

}}}  // namespace boost::stacktrace::impl

