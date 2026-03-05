module;

#include <boost/config.hpp>

#define BOOST_STACKTRACE_INTERFACE_UNIT

export module boost.stacktrace.dump;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/safe_dump_to.hpp>
