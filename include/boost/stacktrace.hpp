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

#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>

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

// Forward declarations
class stacktrace;
namespace detail { class iterator; }


/// Non-owning class that references the frame information stored inside the boost::stacktrace::stacktrace class.
class frame_view {
    /// @cond
    const stacktrace* impl_;
    std::size_t frame_no_;

    frame_view(); // = delete

    frame_view(const stacktrace* impl, std::size_t frame_no) BOOST_NOEXCEPT
        : impl_(impl)
        , frame_no_(frame_no)
    {}

    friend class ::boost::stacktrace::detail::iterator;
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
    std::string name() const;

    /// @returns Address of the frame function.
    /// @throws Nothing.
    const void* address() const BOOST_NOEXCEPT;

    /// @returns Path to the source file, were the function of the frame is defined. Returns empty string
    /// if this->source_line() == 0.
    /// @throws std::bad_alloc if not enough memory to construct resulting string.
    std::string source_file() const;

    /// @returns Code line in the source file, were the function of the frame is defined.
    /// @throws Nothing.
    std::size_t source_line() const  BOOST_NOEXCEPT;
};


/// @cond
namespace detail {
    class iterator: public boost::iterator_facade<
        iterator,
        frame_view,
        boost::random_access_traversal_tag,
        frame_view>
    {
        typedef boost::iterator_facade<
            iterator,
            frame_view,
            boost::random_access_traversal_tag,
            frame_view
        > base_t;

        frame_view f_;

        iterator(const stacktrace* impl, std::size_t frame_no) BOOST_NOEXCEPT
            : f_(impl, frame_no)
        {}

        friend class ::boost::stacktrace::stacktrace;
        friend class ::boost::iterators::iterator_core_access;

        frame_view dereference() const BOOST_NOEXCEPT {
            return f_;
        }

        bool equal(const iterator& it) const BOOST_NOEXCEPT {
            return f_.impl_ == it.f_.impl_ && f_.frame_no_ == it.f_.frame_no_;
        }

        void increment() BOOST_NOEXCEPT {
            ++f_.frame_no_;
        }

        void decrement() BOOST_NOEXCEPT {
            --f_.frame_no_;
        }

        void advance(std::size_t n) BOOST_NOEXCEPT {
            f_.frame_no_ += n;
        }

        base_t::difference_type distance_to(const iterator& it) const {
            BOOST_ASSERT(f_.impl_ == it.f_.impl_);
            return it.f_.frame_no_ - f_.frame_no_;
        }
    };
} // namespace detail
/// @endcond


/// Class that on construction copies minimal information about call stack into its internals and provides access to that information.
class stacktrace {
    /// @cond
#ifdef BOOST_STACKTRACE_LINK
    BOOST_STATIC_CONSTEXPR std::size_t max_implementation_size = sizeof(void*) * 110u;
    boost::aligned_storage<max_implementation_size>::type impl_;
#else
    boost::stacktrace::detail::backtrace_holder impl_;
#endif
    std::size_t hash_code_;

    BOOST_STACKTRACE_FUNCTION std::string get_name(std::size_t frame_no) const;
    BOOST_STACKTRACE_FUNCTION const void* get_address(std::size_t frame_no) const BOOST_NOEXCEPT;
    BOOST_STACKTRACE_FUNCTION std::string get_source_file(std::size_t frame_no) const;
    BOOST_STACKTRACE_FUNCTION std::size_t get_source_line(std::size_t frame_no) const BOOST_NOEXCEPT;
    /// @endcond

    friend class frame_view;
public:
    typedef frame_view                              reference;

    /// @brief Random access iterator that returns frame_view.
    typedef detail::iterator                        iterator;
    typedef iterator                                const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

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

    /// @param frame_no Zero based index of frame to return. 0
    /// is the function index where stacktrace was constructed and
    /// index close to this->size() contains function `main()`.
    /// @returns frame_view that references the actual frame info, stored inside *this.
    ///
    /// @b Complexity: Amortized O(1), O(1) for noop backend.
    frame_view operator[](std::size_t frame_no) const BOOST_NOEXCEPT {
        return *(cbegin() + frame_no);
    }


    /// @b Complexity: O(1)
    const_iterator begin() const BOOST_NOEXCEPT { return const_iterator(this, 0); }
    /// @b Complexity: O(1)
    const_iterator cbegin() const BOOST_NOEXCEPT { return const_iterator(this, 0); }
    /// @b Complexity: O(1)
    const_iterator end() const BOOST_NOEXCEPT { return const_iterator(this, size()); }
    /// @b Complexity: O(1)
    const_iterator cend() const BOOST_NOEXCEPT { return const_iterator(this, size()); }

    /// @b Complexity: O(1)
    const_reverse_iterator rbegin() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(this, 0) ); }
    /// @b Complexity: O(1)
    const_reverse_iterator crbegin() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(this, 0) ); }
    /// @b Complexity: O(1)
    const_reverse_iterator rend() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(this, size()) ); }
    /// @b Complexity: O(1)
    const_reverse_iterator crend() const BOOST_NOEXCEPT { return const_reverse_iterator( const_iterator(this, size()) ); }


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
    std::size_t hash_code() const BOOST_NOEXCEPT { return hash_code_; }

    /// @cond
    bool operator!() const BOOST_NOEXCEPT { return !size(); }
    /// @endcond
};


// frame_view member functions implementation
inline std::string frame_view::name() const {
    return impl_->get_name(frame_no_);
}

inline const void* frame_view::address() const BOOST_NOEXCEPT {
    return impl_->get_address(frame_no_);
}

inline std::string frame_view::source_file() const {
    return impl_->get_source_file(frame_no_);
}

inline std::size_t frame_view::source_line() const  BOOST_NOEXCEPT {
    return impl_->get_source_line(frame_no_);
}


/// Comparison operators that provide platform dependant ordering and have amortized O(1) complexity; O(size()) worst case complexity.
inline bool operator> (const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator!=(const stacktrace& lhs, const stacktrace& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }


/// Comparison operators that provide platform dependant ordering and have O(1) complexity.
inline bool operator< (const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return lhs.address() < rhs.address(); }
inline bool operator> (const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return rhs < lhs; }
inline bool operator<=(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return !(lhs > rhs); }
inline bool operator>=(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return !(lhs < rhs); }
inline bool operator==(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return lhs.address() == rhs.address(); }
inline bool operator!=(const frame_view& lhs, const frame_view& rhs) BOOST_NOEXCEPT { return !(lhs == rhs); }


/// Hashing support, O(1) complexity.
inline std::size_t hash_value(const stacktrace& st) BOOST_NOEXCEPT {
    return st.hash_code();
}

/// Hashing support, O(1) complexity.
inline std::size_t hash_value(const frame_view& f) BOOST_NOEXCEPT {
    return reinterpret_cast<std::size_t>(f.address());
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

/// @cond
#undef BOOST_STACKTRACE_FUNCTION

#ifndef BOOST_STACKTRACE_LINK
#   include <boost/stacktrace/detail/stacktrace.ipp>
#endif
/// @endcond

#endif // BOOST_STACKTRACE_STACKTRACE_HPP
