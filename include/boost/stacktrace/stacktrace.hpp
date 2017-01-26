// Copyright Antony Polukhin, 2016-2017.
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

#include <boost/core/explicit_operator_bool.hpp>

#include <iosfwd>
#include <string>
#include <boost/container/vector.hpp>

#include <boost/stacktrace/stacktrace_fwd.hpp>
#include <boost/stacktrace/frame.hpp>

/// @cond
namespace boost {
    // Forward declaration
    template <class It> std::size_t hash_range(It, It);
}
/// @endcond

namespace boost { namespace stacktrace {

/// Class that on construction copies minimal information about call stack into its internals and provides access to that information.
/// @tparam Allocator Allocator to use during stack capture.
template <class Allocator>
class basic_stacktrace {
    boost::container::vector<frame, Allocator> impl_;

    /// @cond
    static const std::size_t frames_to_skip = 2;

    void fill(void** begin, std::size_t size) {
        if (size < frames_to_skip) {
            return;
        }

        impl_.reserve(static_cast<std::size_t>(size - frames_to_skip));
        for (std::size_t i = frames_to_skip; i < size; ++i) {
            impl_.push_back(
                frame(begin[i])
            );
        }
    }

    template <class Vector, class InIt, class Categ>
    static void try_reserve(Vector& v, InIt, InIt, Categ) {
        v.reserve(boost::stacktrace::detail::max_frames_dump);
    }

    template <class Vector, class InIt>
    static void try_reserve(Vector& v, InIt first, InIt last, std::random_access_iterator_tag) {
        const size_type size = static_cast<size_type>(last - first);
        v.reserve(size > 1024 ? 1024 : size); // Dealing with suspiciously big sizes
    }

    /// @endcond

public:
    typedef typename boost::container::vector<frame, Allocator>::value_type             value_type;
    typedef typename boost::container::vector<frame, Allocator>::allocator_type         allocator_type;
    typedef typename boost::container::vector<frame, Allocator>::const_pointer          pointer;
    typedef typename boost::container::vector<frame, Allocator>::const_pointer          const_pointer;
    typedef typename boost::container::vector<frame, Allocator>::const_reference        reference;
    typedef typename boost::container::vector<frame, Allocator>::const_reference        const_reference;
    typedef typename boost::container::vector<frame, Allocator>::size_type              size_type;
    typedef typename boost::container::vector<frame, Allocator>::difference_type        difference_type;
    typedef typename boost::container::vector<frame, Allocator>::const_iterator         iterator;
    typedef typename boost::container::vector<frame, Allocator>::const_iterator         const_iterator;
    typedef typename boost::container::vector<frame, Allocator>::const_reverse_iterator reverse_iterator;
    typedef typename boost::container::vector<frame, Allocator>::const_reverse_iterator const_reverse_iterator;

    /// @brief Stores the current function call sequence inside the class.
    ///
    /// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
    ///
    /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
    ///
    /// @param max_depth Max call sequence depth to collect.
    ///
    /// @throws Nothing. Note that default construction of allocator may throw, however it is
    /// performed outside the constructor and exception in `allocator_type()` would not result in calling `std::terminate`.
    BOOST_NOINLINE explicit basic_stacktrace(std::size_t max_depth = static_cast<std::size_t>(-1), const allocator_type& a = allocator_type()) BOOST_NOEXCEPT
        : impl_(a)
    {
        BOOST_CONSTEXPR_OR_CONST size_t buffer_size = 128;
        if (!max_depth) {
            return;
        }

        if (static_cast<std::size_t>(-1) - frames_to_skip >= max_depth) {
            max_depth += frames_to_skip;
        }

        try {
            {   // Fast path without additional allocations
                void* buffer[buffer_size];
                const std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(buffer, buffer_size);
                if (buffer_size > frames_count || frames_count >= max_depth) {
                    const std::size_t size = (max_depth < frames_count ? max_depth : frames_count);
                    fill(buffer, size);
                    return;
                }
            }

            // Failed to fit in `buffer_size`. Allocating memory:
            typedef typename Allocator::template rebind<void*>::other allocator_void_t;
            boost::container::vector<void*, allocator_void_t> buf(buffer_size * 2, 0, impl_.get_allocator());
            do {
                const std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(buf.data(), buf.size());
                if (buf.size() > frames_count || frames_count >= max_depth) {
                    const std::size_t size = (max_depth < frames_count ? max_depth : frames_count);
                    fill(buf.data(), size);
                    return;
                }

                buf.resize(buf.size() * 2);
            } while (1);
        } catch (...) {
            // ignore exception
        }
    }

    /// @b Complexity: O(st.size())
    ///
    /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
    basic_stacktrace(const basic_stacktrace& st)
        : impl_(st.impl_)
    {}

    /// @b Complexity: O(st.size())
    ///
    /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
    basic_stacktrace& operator=(const basic_stacktrace& st) {
        impl_ = st.impl_;
        return *this;
    }

#ifdef BOOST_STACKTRACE_DOXYGEN_INVOKED
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe if Allocator::deallocate is async signal safe.
    ~basic_stacktrace() BOOST_NOEXCEPT = default;
#endif

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe if Allocator construction and copying are async signal safe.
    basic_stacktrace(basic_stacktrace&& st) = default;

    /// @b Complexity: O(st.size())
    ///
    /// @b Async-Handler-Safety: Safe if Allocator construction and copying are async signal safe.
    basic_stacktrace& operator=(basic_stacktrace&& st) = default;
#endif

    /// @returns Number of function names stored inside the class.
    ///
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    size_type size() const BOOST_NOEXCEPT {
        return impl_.size();
    }

    /// @param frame_no Zero based index of frame to return. 0
    /// is the function index where stacktrace was constructed and
    /// index close to this->size() contains function `main()`.
    /// @returns frame that references the actual frame info, stored inside *this.
    ///
    /// @b Complexity: Amortized O(1), O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
    ///
    /// @b Async-Handler-Safety: Safe.
    const_reference operator[](std::size_t frame_no) const BOOST_NOEXCEPT {
        return impl_[frame_no];
    }

    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_iterator begin() const BOOST_NOEXCEPT { return impl_.begin(); }
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_iterator cbegin() const BOOST_NOEXCEPT { return impl_.begin(); }
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_iterator end() const BOOST_NOEXCEPT { return impl_.end(); }
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_iterator cend() const BOOST_NOEXCEPT { return impl_.end(); }

    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_reverse_iterator rbegin() const BOOST_NOEXCEPT { return impl_.rbegin(); }
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_reverse_iterator crbegin() const BOOST_NOEXCEPT { return impl_.rbegin(); }
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_reverse_iterator rend() const BOOST_NOEXCEPT { return impl_.rend(); }
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    const_reverse_iterator crend() const BOOST_NOEXCEPT { return impl_.rend(); }


    /// @brief Allows to check that stack trace capturing was successful.
    /// @returns `true` if `this->size() != 0`
    ///
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

    /// @brief Allows to check that stack trace failed.
    /// @returns `true` if `this->size() == 0`
    ///
    /// @b Complexity: O(1)
    ///
    /// @b Async-Handler-Safety: Safe.
    bool empty() const BOOST_NOEXCEPT { return !size(); }

    /// @cond
    bool operator!() const BOOST_NOEXCEPT { return !size(); }
    /// @endcond

    const boost::container::vector<frame, Allocator>& as_vector() const BOOST_NOEXCEPT {
        return impl_;
    }

    /// Constructs stacktrace from basic_istreamable that references the dumped stacktrace.
    ///
    /// @b Complexity: O(N)
    template <class Char, class Trait>
    static basic_stacktrace from_dump(std::basic_istream<Char, Trait>& in, const allocator_type& a = allocator_type()) {
        typedef typename std::basic_istream<Char, Trait>::pos_type pos_type;
        basic_stacktrace ret(0, a);

        // reserving space
        const pos_type pos = in.tellg();
        in.seekg(0, in.end);
        ret.impl_.reserve(in.tellg() / sizeof(void*));
        in.seekg(pos);

        void* ptr = 0;
        while (in.read(reinterpret_cast<Char*>(&ptr), sizeof(ptr))) {
            if (!ptr) {
                break;
            }

            ret.impl_.emplace_back(ptr);
        }

        return ret;
    }

    /// Constructs stacktrace from data pointed by iterator range. Iterators must have either `void*`, `const void*` or `boost::stacktrace::frame` value_type.
    ///
    /// @b Complexity: std::distance(first, last)
    template <class InIt>
    static basic_stacktrace from_dump(InIt first, InIt last, const allocator_type& a = allocator_type()) {
        basic_stacktrace ret(0, a);

        try_reserve(
            ret.impl_,
            first,
            last,
            typename boost::container::iterator_traits<InIt>::iterator_category()
        );

        for (; first != last; ++first) {
            if (!*first) {
                break;
            }

            ret.impl_.emplace_back(*first);
        }

        return ret;
    }
};

/// @brief Compares stacktraces for less, order is platform dependent.
///
/// @b Complexity: Amortized O(1); worst case O(size())
///
/// @b Async-Handler-Safety: Safe.
template <class Allocator1, class Allocator2>
bool operator< (const basic_stacktrace<Allocator1>& lhs, const basic_stacktrace<Allocator2>& rhs) BOOST_NOEXCEPT {
    return lhs.size() < rhs.size() || (lhs.size() == rhs.size() && lhs.as_vector() < rhs.as_vector());
}

/// @brief Compares stacktraces for equality.
///
/// @b Complexity: Amortized O(1); worst case O(size())
///
/// @b Async-Handler-Safety: Safe.
template <class Allocator1, class Allocator2>
bool operator==(const basic_stacktrace<Allocator1>& lhs, const basic_stacktrace<Allocator2>& rhs) BOOST_NOEXCEPT {
    return lhs.as_vector() == rhs.as_vector();
}


/// Comparison operators that provide platform dependant ordering and have amortized O(1) complexity; O(size()) worst case complexity; are Async-Handler-Safe.
template <class Allocator1, class Allocator2>
bool operator> (const basic_stacktrace<Allocator1>& lhs, const basic_stacktrace<Allocator2>& rhs) BOOST_NOEXCEPT {
    return rhs < lhs;
}

template <class Allocator1, class Allocator2>
bool operator<=(const basic_stacktrace<Allocator1>& lhs, const basic_stacktrace<Allocator2>& rhs) BOOST_NOEXCEPT {
    return !(lhs > rhs);
}

template <class Allocator1, class Allocator2>
bool operator>=(const basic_stacktrace<Allocator1>& lhs, const basic_stacktrace<Allocator2>& rhs) BOOST_NOEXCEPT {
    return !(lhs < rhs);
}

template <class Allocator1, class Allocator2>
bool operator!=(const basic_stacktrace<Allocator1>& lhs, const basic_stacktrace<Allocator2>& rhs) BOOST_NOEXCEPT {
    return !(lhs == rhs);
}

/// Hashing support, O(st.size()) complexity; Async-Handler-Safe.
template <class Allocator>
std::size_t hash_value(const basic_stacktrace<Allocator>& st) BOOST_NOEXCEPT {
    return boost::hash_range(st.as_vector().data(), st.as_vector().data()+ st.as_vector().size());
}

/// Outputs stacktrace in a human readable format to output stream; unsafe to use in async handlers.
template <class CharT, class TraitsT, class Allocator>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const basic_stacktrace<Allocator>& bt) {
    return os << boost::stacktrace::detail::to_string(bt.as_vector().data(), bt.size());
}

/// This is the typedef to use unless you'd like to provide a specific allocator to boost::stacktrace::basic_stacktrace.
typedef basic_stacktrace<> stacktrace;

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_STACKTRACE_HPP
