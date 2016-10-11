// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_LIBUNWIND_HPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_LIBUNWIND_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/backtrace_holder_libunwind.hpp>
#include <boost/stacktrace/detail/helpers.hpp>

namespace boost { namespace stacktrace {

stacktrace::stacktrace() BOOST_NOEXCEPT {
    boost::stacktrace::detail::backtrace_holder& bt = boost::stacktrace::detail::construct_bt_and_return(impl_);

    unw_context_t uc;
    if (unw_getcontext(&uc) != 0) {
        return;
    }

    {   // Counting frames_count
        unw_cursor_t cursor;
        if (unw_init_local(&cursor, &uc) != 0) {
            return;
        }
        while (unw_step(&cursor) > 0) {
            ++ bt.frames_count;
        }
    }

    unw_cursor_t cursor;
    if (unw_init_local(&cursor, &uc) != 0) {
        bt.frames_count = 0;
        return;
    }

    BOOST_TRY {
        bt.frames = boost::make_shared<std::string[]>(bt.frames_count);
        std::size_t i = 0;
        while (unw_step(&cursor) > 0){
            bt.frames[i] = boost::stacktrace::detail::backtrace_holder::get_frame_impl(cursor);
            ++ i;
        }
    } BOOST_CATCH(...) {}
    BOOST_CATCH_END
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_LIBUNWIND_HPP
