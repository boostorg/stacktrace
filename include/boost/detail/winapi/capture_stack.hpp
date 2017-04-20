//  Copyright Antony Polukhin 2017.

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DETAIL_WINAPI_CAPTURE_STACK_HPP_
#define BOOST_DETAIL_WINAPI_ENVIRONMENT_HPP_

#include <boost/detail/winapi/basic_types.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {

BOOST_SYMBOL_IMPORT boost::detail::winapi::USHORT_ WINAPI RtlCaptureStackBackTrace(
    boost::detail::winapi::ULONG_  FramesToSkip,
    boost::detail::winapi::ULONG_  FramesToCapture,
    boost::detail::winapi::PVOID_  *BackTrace,
    boost::detail::winapi::PULONG_ BackTraceHash
);

} // extern "C"
#endif // !defined( BOOST_USE_WINDOWS_H )

namespace boost { namespace detail { namespace winapi {

using ::RtlCaptureStackBackTrace;

} // namespace winapi
} // namespace detail
} // namespace boost

#endif // BOOST_DETAIL_WINAPI_CAPTURE_STACK_HPP_
