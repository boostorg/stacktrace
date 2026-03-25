// Copyright Antony Polukhin, 2025-2026.
// Copyright Fedor Osetrov, 2025-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

module;

#include <boost/config.hpp>

#if !defined(BOOST_STACKTRACE_USE_STD_MODULE)
#include <vector>
#include <memory>
#endif

#define BOOST_STACKTRACE_INTERFACE_UNIT
#define BOOST_STACKTRACE_LINK

export module boost.stacktrace.noop;

#if defined(BOOST_STACKTRACE_USE_STD_MODULE)
import std;
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/safe_dump_to.hpp>
#include <boost/stacktrace/stacktrace.hpp>

module :private;
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#include <boost/stacktrace/detail/frame_noop.ipp>
#include <boost/stacktrace/detail/safe_dump_noop.ipp>
#include <boost/stacktrace/detail/collect_noop.ipp>
