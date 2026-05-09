// Copyright Antony Polukhin, 2025-2026.
// Copyright Fedor Osetrov, 2025-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

module;

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <boost/config.hpp>
#include <boost/predef.h>

<<<<<<< HEAD
#if !defined(BOOST_STACKTRACE_USE_STD_MODULE)
#include <stdio.h>
#endif

#include <fcntl.h>
#include <unwind.h>
=======
#include <fcntl.h>
#include <unwind.h>
#include <stdio.h>
>>>>>>> develop
#include <sys/stat.h>

#define BOOST_STACKTRACE_INTERFACE_UNIT
#define BOOST_STACKTRACE_LINK

export module boost.stacktrace.dump;

<<<<<<< HEAD
#if defined(BOOST_STACKTRACE_USE_STD_MODULE)
import std;
#endif

=======
>>>>>>> develop
#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/safe_dump_to.hpp>

module :private;
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
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

