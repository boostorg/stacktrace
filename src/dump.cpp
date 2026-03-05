#if defined(BOOST_USE_MODULES)
module;
#endif

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <boost/config.hpp>
#include <boost/predef.h>

#include <cstddef>

#include <unwind.h>

#define BOOST_STACKTRACE_LINK
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS

#if defined(BOOST_USE_MODULES)
module boost.stacktrace.dump;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#endif

#include <boost/stacktrace/detail/collect_unwind.ipp>
#include <boost/stacktrace/detail/safe_dump_posix.ipp>
