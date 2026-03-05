module;

#include <boost/config.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/core/demangle.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/container_hash/hash_fwd.hpp>
#include <boost/predef.h>

#include <backtrace.h>

#include <array>
#include <iosfwd>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <cxxabi.h>

#define BOOST_STACKTRACE_INTERFACE_UNIT
#define BOOST_STACKTRACE_LINK

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

export module boost.stacktrace.backtrace;

import boost.stacktrace.dump;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/stacktrace.hpp>
#include <boost/stacktrace/this_thread.hpp>

module :private;
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace/detail/frame_unwind.ipp>
