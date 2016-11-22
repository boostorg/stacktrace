// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_CONST_ITERATOR_HPP
#define BOOST_STACKTRACE_CONST_ITERATOR_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>

#include <boost/stacktrace/detail/backend.hpp>
#include <boost/stacktrace/frame_view.hpp>

namespace boost { namespace stacktrace {

// Forward declarations
class stacktrace;

/// Random access iterator over frames that returns `frame_view` on dereference.
class const_iterator: public boost::iterator_facade<
    const_iterator,
    frame_view,
    boost::random_access_traversal_tag,
    frame_view>
{
/// @cond
    typedef boost::iterator_facade<
        const_iterator,
        frame_view,
        boost::random_access_traversal_tag,
        frame_view
    > base_t;

    frame_view f_;

    const_iterator(const boost::stacktrace::detail::backend* impl, std::size_t frame_no) BOOST_NOEXCEPT
        : f_(impl, frame_no)
    {}

    friend class ::boost::stacktrace::stacktrace;
    friend class ::boost::iterators::iterator_core_access;

    frame_view dereference() const BOOST_NOEXCEPT {
        return f_;
    }

    bool equal(const const_iterator& it) const BOOST_NOEXCEPT {
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

    base_t::difference_type distance_to(const const_iterator& it) const {
        BOOST_ASSERT(f_.impl_ == it.f_.impl_);
        return it.f_.frame_no_ - f_.frame_no_;
    }
/// @endcond
};

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_CONST_ITERATOR_HPP
