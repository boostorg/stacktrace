module;

#include <boost/config.hpp>

#include <vector>
#include <memory>

#define BOOST_STACKTRACE_INTERFACE_UNIT
#define BOOST_STACKTRACE_LINK

export module boost.stacktrace.noop;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/safe_dump_to.hpp>
#include <boost/stacktrace/stacktrace.hpp>

module :private;
#include <boost/stacktrace/detail/frame_noop.ipp>
#include <boost/stacktrace/detail/safe_dump_noop.ipp>
#include <boost/stacktrace/detail/collect_noop.ipp>
