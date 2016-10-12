// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_STACKTRACE_HPP
#define BOOST_STACKTRACE_STACKTRACE_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/aligned_storage.hpp>
#include <boost/core/explicit_operator_bool.hpp>
#include <iosfwd>
#include <string>

/// @cond

// Link or header only
#if !defined(BOOST_STACKTRACE_LINK) && defined(BOOST_STACKTRACE_DYN_LINK)
#   define BOOST_STACKTRACE_LINK
#endif

#if defined(BOOST_STACKTRACE_LINK) && !defined(BOOST_STACKTRACE_DYN_LINK) && defined(BOOST_ALL_DYN_LINK)
#   define BOOST_STACKTRACE_DYN_LINK
#endif

// Backend autodetection
#if !defined(BOOST_STACKTRACE_USE_NOOP) && !defined(BOOST_STACKTRACE_USE_WINDBG) && !defined(BOOST_STACKTRACE_USE_LIBUNWIND) \
    && !defined(BOOST_STACKTRACE_USE_BACKTRACE) &&!defined(BOOST_STACKTRACE_USE_HEADER)

#if defined(__has_include) && (!defined(__GNUC__) || __GNUC__ > 4 || BOOST_CLANG)
#   if __has_include(<libunwind.h>)
#       define BOOST_STACKTRACE_USE_LIBUNWIND
#   elif __has_include(<execinfo.h>)
#       define BOOST_STACKTRACE_USE_BACKTRACE
#   elif __has_include("DbgHelp.h")
#       define BOOST_STACKTRACE_USE_WINDBG
#   endif
#else
#   if defined(BOOST_WINDOWS)
#       define BOOST_STACKTRACE_USE_WINDBG
#   else
#       define BOOST_STACKTRACE_USE_BACKTRACE
#   endif
#endif

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
#else
#   define BOOST_STACKTRACE_FUNCTION inline
#   if defined(BOOST_STACKTRACE_USE_NOOP)
#       include <boost/stacktrace/detail/backtrace_holder_noop.hpp>
#   elif defined(BOOST_STACKTRACE_USE_WINDBG)
#      include <boost/stacktrace/detail/backtrace_holder_windows.hpp>
#   elif defined(BOOST_STACKTRACE_USE_LIBUNWIND)
#      include <boost/stacktrace/detail/backtrace_holder_libunwind.hpp>
#   elif defined(BOOST_STACKTRACE_USE_BACKTRACE)
#      include <boost/stacktrace/detail/backtrace_holder_linux.hpp>
#   else
#       error No suitable backtrace backend found
#   endif
#endif
/// @endcond

namespace boost { namespace stacktrace {

class stacktrace {
#ifdef BOOST_STACKTRACE_LINK
    BOOST_STATIC_CONSTEXPR std::size_t max_implementation_size = sizeof(void*) * 110u;
    boost::aligned_storage<max_implementation_size>::type impl_;
#else
    boost::stacktrace::detail::backtrace_holder impl_;
#endif

    std::size_t hash_code_;

public:
    /// @brief Stores the current function call sequence inside the class.
    ///
    /// @b Complexity: O(N) where N is call seaquence length, O(1) for noop backend.
    BOOST_STACKTRACE_FUNCTION stacktrace() BOOST_NOEXCEPT;

    /// @b Complexity: O(1)
    BOOST_STACKTRACE_FUNCTION stacktrace(const stacktrace& bt) BOOST_NOEXCEPT;

    /// @b Complexity: O(1)
    BOOST_STACKTRACE_FUNCTION stacktrace& operator=(const stacktrace& bt) BOOST_NOEXCEPT;

    /// @b Complexity: O(N) for libunwind, O(1) for other backends.
    BOOST_STACKTRACE_FUNCTION ~stacktrace() BOOST_NOEXCEPT;

    /// @returns Number of function names stored inside the class.
    ///
    /// @b Complexity: O(1)
    BOOST_STACKTRACE_FUNCTION std::size_t size() const BOOST_NOEXCEPT;

    /// @param frame Zero based index of function to return. 0
    /// is the function index where stacktrace was constructed and
    /// index close to this->size() contains function `main()`.
    /// @returns Function name in a human readable form.
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    ///
    /// @b Complexity: Amortized O(1), O(1) for noop backend.
    BOOST_STACKTRACE_FUNCTION std::string operator[](std::size_t frame) const;

    /// @cond
    bool operator!() const BOOST_NOEXCEPT {
        return !size();
    }
    /// @endcond

    /// @brief Allows to check that stack trace capturing was successful.
    /// @returns `true` if `this->size() != 0`
    ///
    /// @b Complexity: O(1)
    BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

    /// @brief Compares stacktraces for less, order is platform dependant.
    ///
    /// @b Complexity: Amortized O(1); worst case O(size())
    BOOST_STACKTRACE_FUNCTION bool operator< (const stacktrace& rhs) const BOOST_NOEXCEPT;

    /// @brief Compares stacktraces for equality.
    ///
    /// @b Complexity: Amortized O(1); worst case O(size())
    BOOST_STACKTRACE_FUNCTION bool operator==(const stacktrace& rhs) const BOOST_NOEXCEPT;

    /// @brief Returns hashed code of the stacktrace.
    ///
    /// @b Complexity: O(1)
    std::size_t hash_code() const BOOST_NOEXCEPT {
        return hash_code_;
    }
};

/// Additional comparison operators for stacktraces that have amortized O(1) complexity.
inline bool operator> (const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator!=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }

/// Hashing support
inline std::size_t hash_value(const stacktrace& st) BOOST_NOEXCEPT {
    return st.hash_code();
}

/// Outputs stacktrace in a human readable format to output stream.
template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const stacktrace& bt) {
    const std::streamsize w = os.width();
    const std::size_t frames = bt.size();
    for (std::size_t i = 0; i < frames; ++i) {
        os.width(2);
        os << i;
        os.width(w);
        os << "# ";
        os << bt[i];
        os << '\n';
    }

    return os;
}

}} // namespace boost::stacktrace

/// @cond
#undef BOOST_STACKTRACE_FUNCTION

#ifndef BOOST_STACKTRACE_LINK
#   include <boost/stacktrace/detail/stacktrace.ipp>
#endif
/// @endcond

#endif // BOOST_STACKTRACE_STACKTRACE_HPP
