// Copyright Antony Polukhin, 2016-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(BOOST_USE_MODULES)
module;
#endif

#include <boost/core/demangle.hpp>
#include <boost/predef.h>

#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#define BOOST_STACKTRACE_LINK

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <array>
#include <fstream>
#include <sstream>
#include <exception>
#include <type_traits>

#include <unwind.h>
#include <dlfcn.h>

#if defined(BOOST_USE_MODULES)
module boost.stacktrace.basic;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif
#endif

#include <boost/stacktrace/detail/frame_unwind.ipp>
