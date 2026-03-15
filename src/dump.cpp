// Copyright Antony Polukhin, 2025-2026
// Copyright Fedor Osetrov, 2025-2026
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_STACKTRACE_LINK
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#if defined(BOOST_WINDOWS)
#    include <boost/stacktrace/detail/safe_dump_win.ipp>
#else
#    include <boost/stacktrace/detail/safe_dump_posix.ipp>
#endif
#if defined(BOOST_WINDOWS) && !defined(BOOST_WINAPI_IS_MINGW) // MinGW does not provide RtlCaptureStackBackTrace. MinGW-w64 does.
#    include <boost/stacktrace/detail/collect_msvc.ipp>
#else
#    include <boost/stacktrace/detail/collect_unwind.ipp>
#endif
