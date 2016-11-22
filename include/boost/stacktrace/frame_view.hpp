// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_FRAME_VIEW_HPP
#define BOOST_STACKTRACE_FRAME_VIEW_HPP

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
class frame_view {
    /// @cond
    const boost::stacktrace::detail::backend* impl_;
    std::size_t frame_no_;

    frame_view(); // = delete

    frame_view(const boost::stacktrace::detail::backend* impl, std::size_t frame_no) BOOST_NOEXCEPT
        : impl_(impl)
        , frame_no_(frame_no)
    {}

    friend class ::boost::stacktrace::const_iterator;
    /// @endcond

public:
#ifdef BOOST_STACKTRACE_DOXYGEN_INVOKED
    frame_view() = delete;

    /// @brief Copy constructs frame_view.
    /// @throws Nothing.
    ///
    /// @b Complexity: O(1).
    frame_view(const frame_view&) = default;

    /// @brief Copy assigns frame_view.
    /// @throws Nothing.
    ///
    /// @b Complexity: O(1).
    frame_view& operator=(const frame_view&) = default;
#endif

    /// @returns Name of the frame (function name in a human readable form).
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    std::string name() const {
        return impl_->get_name(frame_no_);
    }

    /// @returns Address of the frame function.
    /// @throws Nothing.
    const void* address() const BOOST_NOEXCEPT {
        return impl_->get_address(frame_no_);
    }

    /// @returns Path to the source file, were the function of the frame is defined. Returns empty string
    /// if this->source_line() == 0.
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    std::string source_file() const {
        return impl_->get_source_file(frame_no_);
    }

    /// @returns Code line in the source file, were the function of the frame is defined.
    /// @throws Nothing.
    std::size_t source_line() const BOOST_NOEXCEPT {
        return impl_->get_source_line(frame_no_);
    }
};

/// Comparison operators that provide platform dependant ordering and have O(1) complexity.
inline bool operator< (const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return lhs.address() < rhs.address(); }
inline bool operator> (const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator==(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return lhs.address() == rhs.address(); }
inline bool operator!=(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }

/// Hashing support, O(1) complexity.
inline std::size_t hash_value(const frame_view& f) BOOST_NOEXCEPT {
    return reinterpret_cast<std::size_t>(f.address());
}

/// Outputs stacktrace::frame in a human readable format to output stream.
template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const frame_view& f) {
    os << f.name();

    if (f.source_line()) {
        return os << '\t' << f.source_file() << ':' << f.source_line();
    }

    return os;
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_FRAME_VIEW_HPP
