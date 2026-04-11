// Copyright Antony Polukhin, 2025-2026.
// Copyright Fedor Osetrov, 2025-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

module;

#include <boost/config.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/core/demangle.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/container_hash/hash_fwd.hpp>
#include <boost/predef.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <cxxabi.h>

#include <dlfcn.h>

#define BOOST_STACKTRACE_INTERFACE_UNIT
#define BOOST_STACKTRACE_LINK

export module boost.stacktrace.addr2line;

import boost.stacktrace.dump;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/stacktrace/stacktrace.hpp>
#include <boost/stacktrace/this_thread.hpp>

module :private;
#include <boost/stacktrace/detail/frame_unwind.ipp>
