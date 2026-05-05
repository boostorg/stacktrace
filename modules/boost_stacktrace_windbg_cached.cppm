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

#define BOOST_STACKTRACE_INTERFACE_UNIT
#define BOOST_STACKTRACE_LINK

export module boost.stacktrace.windbg;

import boost.stacktrace.dump;

#include <boost/stacktrace/stacktrace.hpp>
#include <boost/stacktrace/this_thread.hpp>

module :private;
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#define BOOST_STACKTRACE_USE_WINDBG_CACHED
#include <boost/stacktrace/detail/frame_msvc.ipp>
