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

#include <boost/stacktrace/detail/backend.hpp>
#include <boost/stacktrace/frame.hpp>
#include <boost/stacktrace/const_iterator.hpp>

namespace boost { namespace stacktrace {

/// Class that on construction copies minimal information about call stack into its internals and provides access to that information.
class stacktrace {
    /// @cond
    BOOST_STATIC_CONSTEXPR std::size_t max_implementation_size = sizeof(void*) * 110u;
    boost::aligned_storage<max_implementation_size>::type impl_;
    std::size_t hash_code_;
    boost::stacktrace::detail::backend back_;
    /// @endcond

public:
    typedef frame                              reference;

    /// @brief Random access iterator that returns frame.
    typedef boost::stacktrace::const_iterator       iterator;
    typedef iterator                                const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    /// @brief Stores the current function call sequence inside the class.
    ///
    /// @b Complexity: O(N) where N is call seaquence length, O(1) for noop backend.
    BOOST_FORCEINLINE stacktrace() BOOST_NOEXCEPT
        : impl_()
        , hash_code_()
        , back_(&impl_, sizeof(impl_), hash_code_)
    {}

    /// @b Complexity: O(1)
    stacktrace(const stacktrace& st) BOOST_NOEXCEPT
        : impl_()
        , hash_code_(st.hash_code_)
        , back_(st.back_, &impl_)
    {}

    /// @b Complexity: O(1)
    stacktrace& operator=(const stacktrace& st) BOOST_NOEXCEPT {
        hash_code_ = st.hash_code_;
        back_ = back_;

        return *this;
    }

    /// @b Complexity: O(N) for libunwind, O(1) for other backends.
    ~stacktrace() BOOST_NOEXCEPT {}

    /// @returns Number of function names stored inside the class.
    ///
    /// @b Complexity: O(1)
    std::size_t size() const BOOST_NOEXCEPT {
        return back_.size();
    }

    /// @param frame_no Zero based index of frame to return. 0
    /// is the function index where stacktrace was constructed and
    /// index close to this->size() contains function `main()`.
    /// @returns frame that references the actual frame info, stored inside *this.
    ///
    /// @b Complexity: Amortized O(1), O(1) for noop backend.
    frame operator[](std::size_t frame_no) const BOOST_NOEXCEPT {
        return *(cbegin() + frame_no);
    }


    /// @b Complexity: O(1)
    const_iterator begin() const BOOST_NOEXCEPT { return const_iterator(&back_, 0); }
    /// @b Complexity: O(1)
    const_iterator cbegin() const BOOST_NOEXCEPT { return const_iterator(&back_, 0); }
    /// @b Complexity: O(1)
    const_iterator end() const BOOST_NOEXCEPT { return const_iterator(&back_, size()); }
    /// @b Complexity: O(1)
    const_iterator cend() const BOOST_NOEXCEPT { return const_iterator(&back_, size()); }

    /// @b Complexity: O(1)
    const_reverse_iterator rbegin() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(&back_, 0) ); }
    /// @b Complexity: O(1)
    const_reverse_iterator crbegin() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(&back_, 0) ); }
    /// @b Complexity: O(1)
    const_reverse_iterator rend() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(&back_, size()) ); }
    /// @b Complexity: O(1)
    const_reverse_iterator crend() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(&back_, size()) ); }


    /// @brief Allows to check that stack trace capturing was successful.
    /// @returns `true` if `this->size() != 0`
    ///
    /// @b Complexity: O(1)
    BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

    /// @brief Compares stacktraces for less, order is platform dependant.
    ///
    /// @b Complexity: Amortized O(1); worst case O(size())
    bool operator< (const stacktrace& rhs) const BOOST_NOEXCEPT {
        return hash_code_ < rhs.hash_code_ || (hash_code_ == rhs.hash_code_ && back_ < rhs.back_);
    }

    /// @brief Compares stacktraces for equality.
    ///
    /// @b Complexity: Amortized O(1); worst case O(size())
    bool operator==(const stacktrace& rhs) const BOOST_NOEXCEPT {
        return hash_code_ == rhs.hash_code_ && back_ == rhs.back_;
    }

    /// @brief Returns hashed code of the stacktrace.
    ///
    /// @b Complexity: O(1)
    std::size_t hash_code() const BOOST_NOEXCEPT { return hash_code_; }

    /// @cond
    bool operator!() const BOOST_NOEXCEPT { return !size(); }
    /// @endcond
};


/// Comparison operators that provide platform dependant ordering and have amortized O(1) complexity; O(size()) worst case complexity.
inline bool operator> (const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator!=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }

/// Hashing support, O(1) complexity.
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

#endif // BOOST_STACKTRACE_STACKTRACE_HPP
