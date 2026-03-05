module;

#include <boost/config.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/core/demangle.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/container_hash/hash_fwd.hpp>
#include <boost/predef.h>

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

export module boost.stacktrace.basic;

import boost.stacktrace.dump;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/stacktrace.hpp>
#include <boost/stacktrace/this_thread.hpp>
