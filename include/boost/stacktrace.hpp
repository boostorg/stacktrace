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

#include <boost/stacktrace/detail/backend.hpp>

namespace boost { namespace stacktrace {

// Forward declarations
class stacktrace;
namespace detail { class iterator; }


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

        iterator(const boost::stacktrace::detail::backend* impl, std::size_t frame_no) BOOST_NOEXCEPT
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
    BOOST_STATIC_CONSTEXPR std::size_t max_implementation_size = sizeof(void*) * 110u;
    boost::aligned_storage<max_implementation_size>::type impl_;
    std::size_t hash_code_;
    boost::stacktrace::detail::backend back_;

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
    stacktrace() BOOST_NOEXCEPT
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
        back_.~backend();
        new (&back_) boost::stacktrace::detail::backend(st.back_, &impl_);
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
    /// @returns frame_view that references the actual frame info, stored inside *this.
    ///
    /// @b Complexity: Amortized O(1), O(1) for noop backend.
    frame_view operator[](std::size_t frame_no) const BOOST_NOEXCEPT {
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

#endif // BOOST_STACKTRACE_STACKTRACE_HPP
