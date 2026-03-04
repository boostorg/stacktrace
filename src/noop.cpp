// Copyright Antony Polukhin, 2016-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(BOOST_STACKTRACE_INTERFACE_UNIT)
module;
#endif
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#define BOOST_STACKTRACE_LINK

#include <boost/stacktrace/detail/frame_noop.ipp>
#include <boost/stacktrace/detail/safe_dump_noop.ipp>

#if defined(BOOST_STACKTRACE_INTERFACE_UNIT)
module boost.stacktrace.noop;
#endif
