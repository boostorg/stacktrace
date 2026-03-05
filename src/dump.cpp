#define BOOST_STACKTRACE_LINK
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <boost/stacktrace/detail/collect_unwind.ipp>
#include <boost/stacktrace/detail/safe_dump_posix.ipp>
