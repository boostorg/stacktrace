module;

#include <boost/config.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/container_hash/hash_fwd.hpp>

// #include <array>
// #include <iosfwd>
// #include <string>
// #include <algorithm>
// #include <vector>
// #include <memory>
// #include <fstream>
// #include <sstream>
#include <string>
#include <type_traits>
#include <exception>
// #include <version>
// #include <utility>
#include <cxxabi.h>
// #include <unistd.h>

#define BOOST_STACKTRACE_INTERFACE_UNIT

export module boost.stacktrace;

import std;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace.hpp>
