// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_FRAME_HPP
#define BOOST_STACKTRACE_FRAME_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <iosfwd>
#include <string>

#include <boost/core/explicit_operator_bool.hpp>

// Link or header only
#if !defined(BOOST_STACKTRACE_LINK) && defined(BOOST_STACKTRACE_DYN_LINK)
#   define BOOST_STACKTRACE_LINK
#endif

#if defined(BOOST_STACKTRACE_LINK) && !defined(BOOST_STACKTRACE_DYN_LINK) && defined(BOOST_ALL_DYN_LINK)
#   define BOOST_STACKTRACE_DYN_LINK
#endif

#ifdef BOOST_STACKTRACE_LINK
#   if defined(BOOST_STACKTRACE_DYN_LINK)
#       ifdef BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#           define BOOST_STACKTRACE_FUNCTION BOOST_SYMBOL_EXPORT
#       else
#           define BOOST_STACKTRACE_FUNCTION BOOST_SYMBOL_IMPORT
#       endif
#   else
#       define BOOST_STACKTRACE_FUNCTION
#   endif
#elif !defined(BOOST_STACKTRACE_DOXYGEN_INVOKED)
#   define BOOST_STACKTRACE_FUNCTION inline
#endif

namespace boost { namespace stacktrace {

class frame;


namespace detail {
    BOOST_STACKTRACE_FUNCTION std::string to_string(const frame* frames, std::size_t size);

    enum helper{ max_frames_dump = 128 };
    BOOST_STACKTRACE_FUNCTION std::size_t from_dump(const char* filename, void** frames);
    BOOST_STACKTRACE_FUNCTION std::size_t dump(const char* file, void** memory, std::size_t size) BOOST_NOEXCEPT;
#if defined(BOOST_WINDOWS)
    BOOST_STACKTRACE_FUNCTION std::size_t dump(void* fd, void** memory, std::size_t size) BOOST_NOEXCEPT;
#else
    // POSIX
    BOOST_STACKTRACE_FUNCTION std::size_t dump(int fd, void** memory, std::size_t size) BOOST_NOEXCEPT;
#endif


struct this_thread_frames { // struct is required to avoid warning about usage of inline+BOOST_NOINLINE
    BOOST_NOINLINE BOOST_STACKTRACE_FUNCTION static std::size_t collect(void** memory, std::size_t size, std::size_t skip) BOOST_NOEXCEPT;
};

} // namespace detail

/// Non-owning class that references the frame information stored inside the boost::stacktrace::stacktrace class.
class frame {
    /// @cond
    const void* addr_;
    /// @endcond

public:
    /// @brief Constructs frame that references NULL address.
    /// Calls to source_file() and source_line() wil lreturn empty string.
    /// Calls to source_line() will return 0.
    ///
    /// @b Complexity: O(1).
    ///
    /// @b Async-Handler-Safety: Safe.
    /// @throws Nothing.
    frame() BOOST_NOEXCEPT
        : addr_(0)
    {}

#ifdef BOOST_STACKTRACE_DOXYGEN_INVOKED
    /// @brief Copy constructs frame.
    ///
    /// @b Complexity: O(1).
    ///
    /// @b Async-Handler-Safety: Safe.
    /// @throws Nothing.
    frame(const frame&) = default;

    /// @brief Copy assigns frame.
    ///
    /// @b Complexity: O(1).
    ///
    /// @b Async-Handler-Safety: Safe.
    /// @throws Nothing.
    frame& operator=(const frame&) = default;
#endif

    /// @brief Constructs frame that can extract information from addr at runtime.
    ///
    /// @b Complexity: O(1).
    ///
    /// @b Async-Handler-Safety: Safe.
    /// @throws Nothing.
    explicit frame(const void* addr) BOOST_NOEXCEPT
        : addr_(addr)
    {}

    /// @returns Name of the frame (function name in a human readable form).
    ///
    /// @b Async-Handler-Safety: Unsafe.
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    BOOST_STACKTRACE_FUNCTION std::string name() const;

    /// @returns Address of the frame function.
    ///
    /// @b Async-Handler-Safety: Safe.
    /// @throws Nothing.
    const void* address() const BOOST_NOEXCEPT {
        return addr_;
    }

    /// @returns Path to the source file, were the function of the frame is defined. Returns empty string
    /// if this->source_line() == 0.
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    ///
    /// @b Async-Handler-Safety: Unsafe.
    BOOST_STACKTRACE_FUNCTION std::string source_file() const;

    /// @returns Code line in the source file, were the function of the frame is defined.
    /// @throws std::bad_alloc if not enough memory to construct string for internal needs.
    ///
    /// @b Async-Handler-Safety: Unsafe.
    BOOST_STACKTRACE_FUNCTION std::size_t source_line() const;

    /// @brief Checks that frame is not references NULL address.
    /// @returns `true` if `this->address() != 0`
    ///
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

    /// @brief Checks that frame references NULL address.
    /// @returns `true` if `this->address() == 0`
    ///
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    bool empty() const BOOST_NOEXCEPT { return !address(); }
    
    /// @cond
    bool operator!() const BOOST_NOEXCEPT { return !address(); }
    /// @endcond
};

/// Comparison operators that provide platform dependant ordering and have O(1) complexity; are Async-Handler-Safe.
inline bool operator< (const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return lhs.address() < rhs.address(); }
inline bool operator> (const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator==(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return lhs.address() == rhs.address(); }
inline bool operator!=(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }

/// Hashing support, O(1) complexity; Async-Handler-Safe.
inline std::size_t hash_value(const frame& f) BOOST_NOEXCEPT {
    return reinterpret_cast<std::size_t>(f.address());
}

/// Outputs stacktrace::frame in a human readable format to string; unsafe to use in async handlers.
BOOST_STACKTRACE_FUNCTION std::string to_string(const frame& f);

/// Outputs stacktrace::frame in a human readable format to output stream; unsafe to use in async handlers.
template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const frame& f) {
    return os << boost::stacktrace::to_string(f);
}

}} // namespace boost::stacktrace

/// @cond

#undef BOOST_STACKTRACE_FUNCTION

#ifndef BOOST_STACKTRACE_LINK
#   if defined(BOOST_STACKTRACE_USE_NOOP)
#       include <boost/stacktrace/detail/frame_noop.ipp>
#   elif defined(BOOST_WINDOWS)
#       include <boost/stacktrace/detail/frame_msvc.ipp>
#   else
#       include <boost/stacktrace/detail/frame_unwind.ipp>
#   endif
#endif
/// @endcond


#endif // BOOST_STACKTRACE_FRAME_HPP
