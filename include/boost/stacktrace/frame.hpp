// Copyright Antony Polukhin, 2016.
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

#include <boost/stacktrace/detail/backend.hpp>

namespace boost { namespace stacktrace {

// Forward declarations
class const_iterator;

/// Non-owning class that references the frame information stored inside the boost::stacktrace::stacktrace class.
class frame {
    /// @cond
    const void* addr_;

    frame(); // = delete
    /// @endcond

public:
#ifdef BOOST_STACKTRACE_DOXYGEN_INVOKED
    frame() = delete;

    /// @brief Copy constructs frame.
    /// @throws Nothing.
    ///
    /// @b Complexity: O(1).
    frame(const frame&) = default;

    /// @brief Copy assigns frame.
    /// @throws Nothing.
    ///
    /// @b Complexity: O(1).
    frame& operator=(const frame&) = default;
#endif

    /// @brief Constructs frame that can extract information from addr at runtime.
    /// @throws Nothing.
    ///
    /// @b Complexity: O(1).
    explicit frame(const void* addr) BOOST_NOEXCEPT
        : addr_(addr)
    {}

    /// @returns Name of the frame (function name in a human readable form).
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    std::string name() const {
        return boost::stacktrace::detail::backend::get_name(address());
    }

    /// @returns Address of the frame function.
    /// @throws Nothing.
    const void* address() const BOOST_NOEXCEPT {
        return addr_;
    }

    /// @returns Path to the source file, were the function of the frame is defined. Returns empty string
    /// if this->source_line() == 0.
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    std::string source_file() const {
        return boost::stacktrace::detail::backend::get_source_file(address());
    }

    /// @returns Code line in the source file, were the function of the frame is defined.
    /// @throws std::bad_alloc if not enough memory to construct string for internal needs.
    std::size_t source_line() const {
        return boost::stacktrace::detail::backend::get_source_line(address());
    }
};

/// Comparison operators that provide platform dependant ordering and have O(1) complexity.
inline bool operator< (const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return lhs.address() < rhs.address(); }
inline bool operator> (const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator==(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return lhs.address() == rhs.address(); }
inline bool operator!=(const frame& lhs, const frame& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }

/// Hashing support, O(1) complexity.
inline std::size_t hash_value(const frame& f) BOOST_NOEXCEPT {
    return reinterpret_cast<std::size_t>(f.address());
}

/// Outputs stacktrace::frame in a human readable format to output stream.
template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const frame& f) {
    os << f.name();

    if (f.source_line()) {
        return os << '\t' << f.source_file() << ':' << f.source_line();
    }

    return os;
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_FRAME_HPP
