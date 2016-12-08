// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_STACKTRACE_FWD_HPP
#define BOOST_STACKTRACE_STACKTRACE_FWD_HPP

/// @file stacktrace_fwd.hpp This header contains only forward declarations of
/// boost::stacktrace::frame, boost::stacktrace::const_iterator, boost::stacktrace::basic_stacktrace
/// and does not include any other headers.

/// @cond
namespace boost { namespace stacktrace {

class frame;

class const_iterator;

template <std::size_t Depth>
class basic_stacktrace;

}} // namespace boost::stacktrace
/// @endcond


#endif // BOOST_STACKTRACE_STACKTRACE_FWD_HPP
