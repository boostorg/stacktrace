// Copyright Antony Polukhin, 2016-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(BOOST_STACKTRACE_INTERFACE_UNIT)
module;
#endif // defined(BOOST_STACKTRACE_INTERFACE_UNIT)

#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#define BOOST_STACKTRACE_USE_ADDR2LINE
#define BOOST_STACKTRACE_LINK

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <boost/stacktrace/detail/frame_unwind.ipp>
#include <boost/stacktrace/safe_dump_to.hpp>

#if defined(BOOST_STACKTRACE_INTERFACE_UNIT)
module boost.stacktrace;
#endif // defined(BOOST_STACKTRACE_INTERFACE_UNIT)
