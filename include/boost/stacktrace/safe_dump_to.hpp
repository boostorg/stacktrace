// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_SAFE_DUMP_TO_HPP
#define BOOST_STACKTRACE_SAFE_DUMP_TO_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>


/// @file safe_dump_to.hpp This header contains low-level async-signal-safe functions for dumping call stacks. Dumps are binary serialized arrays of `void*`,
/// so you could read them by using 'od -tx8 -An stacktrace_dump_failename' Linux command or using boost::stacktrace::stacktrace::from_dump functions.

namespace boost { namespace stacktrace {


/// @cond
namespace detail {
    struct suppress_noinline_warnings {
        BOOST_NOINLINE static std::size_t safe_dump_to_impl(void* memory, std::size_t size, std::size_t skip) BOOST_NOEXCEPT {
            void** mem = static_cast<void**>(memory);
            const std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(mem, size / sizeof(void*) - 1, skip + 1);
            mem[frames_count] = 0;
            return frames_count + 1;
        }

        template <class T>
        BOOST_NOINLINE static std::size_t safe_dump_to_impl(T file, std::size_t skip, std::size_t max_depth) BOOST_NOEXCEPT {
            void* buffer[boost::stacktrace::detail::max_frames_dump + 1];
            if (max_depth > boost::stacktrace::detail::max_frames_dump) {
                max_depth = boost::stacktrace::detail::max_frames_dump;
            }

            const std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(buffer, max_depth, skip + 1);
            buffer[frames_count] = 0;
            return boost::stacktrace::detail::dump(file, buffer, frames_count + 1);
        }
    };
}
/// @endcond

/// @brief Stores current function call sequence into the memory.
///
/// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
///
/// @b Async-Handler-Safety: Safe.
///
/// @returns Stored call sequence depth.
///
/// @param memory Preallocated buffer to store current function call sequence into.
///
/// @param size Size of the preallocated buffer.
BOOST_FORCEINLINE std::size_t safe_dump_to(void* memory, std::size_t size) BOOST_NOEXCEPT {
    return  boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(memory, size, 0);
}

/// @brief Stores current function call sequence into the memory.
///
/// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
///
/// @b Async-Handler-Safety: Safe.
///
/// @returns Stored call sequence depth.
///
/// @param skip How many top calls to skip and do not store.
///
/// @param memory Preallocated buffer to store current function call sequence into.
///
/// @param size Size of the preallocated buffer.
BOOST_FORCEINLINE std::size_t safe_dump_to(std::size_t skip, void* memory, std::size_t size) BOOST_NOEXCEPT {
    return  boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(memory, size, skip);
}


/// @brief Opens a file and rewrites its content with current function call sequence.
///
/// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
///
/// @b Async-Handler-Safety: Safe.
///
/// @returns Stored call sequence depth.
///
/// @param file File to store current function call sequence.
BOOST_FORCEINLINE std::size_t safe_dump_to(const char* file) BOOST_NOEXCEPT {
    return boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(file, 0, boost::stacktrace::detail::max_frames_dump);
}

/// @brief Opens a file and rewrites its content with current function call sequence.
///
/// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
///
/// @b Async-Handler-Safety: Safe.
///
/// @returns Stored call sequence depth.
///
/// @param skip How many top calls to skip and do not store.
///
/// @param max_depth Max call sequence depth to collect.
///
/// @param file File to store current function call sequence.
BOOST_FORCEINLINE std::size_t safe_dump_to(std::size_t skip, std::size_t max_depth, const char* file) BOOST_NOEXCEPT {
    return boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(file, skip, max_depth);
}

#ifdef BOOST_STACKTRACE_DOXYGEN_INVOKED

/// @brief Writes into the provided file descriptor the current function call sequence.
///
/// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
///
/// @b Async-Handler-Safety: Safe.
///
/// @returns Stored call sequence depth.
///
/// @param file File to store current function call sequence.
BOOST_FORCEINLINE std::size_t safe_dump_to(platform_specific_descriptor fd) BOOST_NOEXCEPT;

/// @brief Writes into the provided file descriptor the current function call sequence.
///
/// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
///
/// @b Async-Handler-Safety: Safe.
///
/// @returns Stored call sequence depth.
///
/// @param skip How many top calls to skip and do not store.
///
/// @param max_depth Max call sequence depth to collect.
///
/// @param file File to store current function call sequence.
BOOST_FORCEINLINE std::size_t safe_dump_to(std::size_t skip, std::size_t max_depth, platform_specific_descriptor fd) BOOST_NOEXCEPT;

#elif defined(BOOST_WINDOWS)

BOOST_FORCEINLINE std::size_t safe_dump_to(void* fd) BOOST_NOEXCEPT {
    return boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(fd, 0, boost::stacktrace::detail::max_frames_dump);
}

BOOST_FORCEINLINE std::size_t safe_dump_to(std::size_t skip, std::size_t max_depth, void* fd) BOOST_NOEXCEPT {
    return boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(fd, skip, max_depth);
}

#else

// POSIX
BOOST_FORCEINLINE std::size_t safe_dump_to(int fd) BOOST_NOEXCEPT {
    return boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(fd, 0, boost::stacktrace::detail::max_frames_dump);
}

BOOST_FORCEINLINE std::size_t safe_dump_to(std::size_t skip, std::size_t max_depth, int fd) BOOST_NOEXCEPT {
    return boost::stacktrace::detail::suppress_noinline_warnings::safe_dump_to_impl(fd, skip, max_depth);
}

#endif


}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_SAFE_DUMP_TO_HPP
