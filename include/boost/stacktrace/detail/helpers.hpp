// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_HELPERS_HPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_HELPERS_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder;

template <class T>
inline boost::stacktrace::detail::backtrace_holder& to_bt(T& data) BOOST_NOEXCEPT {
    return *reinterpret_cast<boost::stacktrace::detail::backtrace_holder*>(&data);
}

template <class T>
inline const boost::stacktrace::detail::backtrace_holder& to_bt(const T& data) BOOST_NOEXCEPT {
    return *reinterpret_cast<const boost::stacktrace::detail::backtrace_holder*>(&data);
}

template <class T>
inline boost::stacktrace::detail::backtrace_holder& construct_bt_and_return(T& data) BOOST_NOEXCEPT {
    new (&data) boost::stacktrace::detail::backtrace_holder();
    return boost::stacktrace::detail::to_bt(data);
}

inline boost::stacktrace::detail::backtrace_holder& construct_bt_and_return(backtrace_holder& data) BOOST_NOEXCEPT {
    return data;
}

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_HELPERS_HPP
