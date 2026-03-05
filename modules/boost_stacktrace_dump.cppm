module;

#include <boost/config.hpp>
#include <boost/predef.h>

#define BOOST_STACKTRACE_INTERFACE_UNIT

export module boost.stacktrace.dump;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/safe_dump_to.hpp>

module :private;

#include <boost/stacktrace/detail/collect_unwind.ipp>
#include <boost/stacktrace/detail/safe_dump_posix.ipp>
