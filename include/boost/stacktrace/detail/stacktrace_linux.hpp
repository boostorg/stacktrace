// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_LINUX_HPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_LINUX_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/backtrace_holder_linux.hpp>
#include <boost/stacktrace/detail/helpers.hpp>

#include <boost/functional/hash.hpp>

namespace boost { namespace stacktrace {

stacktrace::stacktrace() BOOST_NOEXCEPT
    : hash_code_(0)
{
    boost::stacktrace::detail::backtrace_holder& bt = boost::stacktrace::detail::construct_bt_and_return(impl_);
    bt.frames_count = ::backtrace(bt.buffer, boost::stacktrace::detail::backtrace_holder::max_size);
    if (bt.buffer[bt.frames_count - 1] == 0) {
        -- bt.frames_count;
    }

    hash_code_ = boost::hash_range(bt.buffer, bt.buffer + bt.frames_count);
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_LINUX_HPP
