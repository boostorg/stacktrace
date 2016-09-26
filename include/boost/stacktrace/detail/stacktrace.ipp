// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_IPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/static_assert.hpp>

// Autodetection
#if !defined(BOOST_STACKTRACE_USE_NOOP) && !defined(BOOST_STACKTRACE_USE_WINDBG) && !defined(BOOST_STACKTRACE_USE_LIBUNWIND) \
    && !defined(BOOST_STACKTRACE_USE_BACKTRACE) &&!defined(BOOST_STACKTRACE_USE_HEADER)

#if defined(__has_include) && (!defined(__GNUC__) || __GNUC__ > 4 || BOOST_CLANG)
#   if __has_include(<libunwind.h>)
#       define BOOST_STACKTRACE_USE_LIBUNWIND
#   elif __has_include(<execinfo.h>)
#       define BOOST_STACKTRACE_USE_BACKTRACE
#   elif __has_include("DbgHelp.h")
#       define BOOST_STACKTRACE_USE_WINDBG
#   endif
#else
#   if defined(BOOST_WINDOWS)
#       define BOOST_STACKTRACE_USE_WINDBG
#   else
#       define BOOST_STACKTRACE_USE_BACKTRACE
#   endif
#endif

#endif


#if defined(BOOST_STACKTRACE_USE_HEADER)
#   include BOOST_STACKTRACE_USE_HEADER
#elif defined(BOOST_STACKTRACE_USE_NOOP)
#   include <boost/stacktrace/detail/stacktrace_noop.hpp>
#elif defined(BOOST_STACKTRACE_USE_WINDBG)
#   include <boost/stacktrace/detail/stacktrace_windows.hpp>
#elif defined(BOOST_STACKTRACE_USE_LIBUNWIND)
#   include <boost/stacktrace/detail/stacktrace_libunwind.hpp>
#elif defined(BOOST_STACKTRACE_USE_BACKTRACE)
#   include <boost/stacktrace/detail/stacktrace_linux.hpp>
#else
#   error No suitable backtrace backend found
#endif



namespace boost { namespace stacktrace {

// stacktrace::stacktrace() is defined in each backend separately. This is
// requered to avoid `boost::stacktrace::detail::backtrace_holder` apearing in
// stack traces.

stacktrace::stacktrace(const stacktrace& bt) BOOST_NOEXCEPT {
    new (&impl_) boost::stacktrace::detail::backtrace_holder(
        boost::stacktrace::detail::to_bt(bt.impl_)
    );
}

stacktrace& stacktrace::operator=(const stacktrace& bt) BOOST_NOEXCEPT {
    boost::stacktrace::detail::to_bt(impl_) = boost::stacktrace::detail::to_bt(bt.impl_);
    return *this;
}

stacktrace::~stacktrace() BOOST_NOEXCEPT {
    BOOST_STATIC_ASSERT_MSG(sizeof(impl_) >= sizeof(boost::stacktrace::detail::backtrace_holder), "Too small storage for holding backtrace");
    boost::stacktrace::detail::to_bt(impl_).~backtrace_holder();
}

std::size_t stacktrace::size() const BOOST_NOEXCEPT {
    return boost::stacktrace::detail::to_bt(impl_).size();
}

std::string stacktrace::operator[](std::size_t frame) const {
    return boost::stacktrace::detail::to_bt(impl_).get_frame(frame);
}


}}

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_IPP
