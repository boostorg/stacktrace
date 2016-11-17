// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_SRC_STACKTRACE_IPP
#define BOOST_STACKTRACE_SRC_STACKTRACE_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace.hpp>
#include <boost/static_assert.hpp>


#if defined(BOOST_STACKTRACE_USE_HEADER)
#   include BOOST_STACKTRACE_USE_HEADER
#elif defined(BOOST_STACKTRACE_USE_NOOP)
#   include <boost/stacktrace/detail/backtrace_holder_noop.hpp>
#   include <boost/stacktrace/detail/stacktrace_noop.hpp>
#elif defined(BOOST_STACKTRACE_USE_WINDBG)
#   include <boost/stacktrace/detail/backtrace_holder_windows.hpp>
#   include <boost/stacktrace/detail/stacktrace_windows.hpp>
#elif defined(BOOST_STACKTRACE_USE_LIBUNWIND)
#   include <boost/stacktrace/detail/backtrace_holder_libunwind.hpp>
#   include <boost/stacktrace/detail/stacktrace_libunwind.hpp>
#elif defined(BOOST_STACKTRACE_USE_BACKTRACE)
#   include <boost/stacktrace/detail/backtrace_holder_linux.hpp>
#   include <boost/stacktrace/detail/stacktrace_linux.hpp>
#else
#   error No suitable backtrace backend found
#endif

#include <boost/stacktrace/detail/helpers.hpp>


namespace boost { namespace stacktrace {

// stacktrace::stacktrace() is defined in each backend separately. This is
// requered to avoid `boost::stacktrace::detail::backtrace_holder` apearing in
// stack traces.

stacktrace::stacktrace(const stacktrace& bt) BOOST_NOEXCEPT
    : hash_code_(bt.hash_code_)
{
    new (&impl_) boost::stacktrace::detail::backtrace_holder(
        boost::stacktrace::detail::to_bt(bt.impl_)
    );
}

stacktrace& stacktrace::operator=(const stacktrace& bt) BOOST_NOEXCEPT {
    boost::stacktrace::detail::to_bt(impl_) = boost::stacktrace::detail::to_bt(bt.impl_);
    hash_code_ = bt.hash_code_;
    return *this;
}

stacktrace::~stacktrace() BOOST_NOEXCEPT {
    BOOST_STATIC_ASSERT_MSG(sizeof(impl_) >= sizeof(boost::stacktrace::detail::backtrace_holder), "Too small storage for holding backtrace");
    boost::stacktrace::detail::to_bt(impl_).~backtrace_holder();
}

std::size_t stacktrace::size() const BOOST_NOEXCEPT {
    return boost::stacktrace::detail::to_bt(impl_).size();
}

std::string stacktrace::get_name(std::size_t frame) const {
    return boost::stacktrace::detail::to_bt(impl_).get_frame(frame);
}

const void* stacktrace::get_address(std::size_t frame) const BOOST_NOEXCEPT {
    return boost::stacktrace::detail::to_bt(impl_).get_address(frame);
}

std::string stacktrace::get_source_file(std::size_t frame) const {
    return boost::stacktrace::detail::to_bt(impl_).get_source_file(frame);
}

std::size_t stacktrace::get_source_line(std::size_t frame) const BOOST_NOEXCEPT {
    return boost::stacktrace::detail::to_bt(impl_).get_source_line(frame);
}

bool stacktrace::operator< (const stacktrace& rhs) const BOOST_NOEXCEPT {
    return hash_code_ < rhs.hash_code_
        || (hash_code_ == rhs.hash_code_ && boost::stacktrace::detail::to_bt(impl_) < boost::stacktrace::detail::to_bt(rhs.impl_))
    ;
}

bool stacktrace::operator==(const stacktrace& rhs) const BOOST_NOEXCEPT {
    return hash_code_ == rhs.hash_code_
        && boost::stacktrace::detail::to_bt(impl_) == boost::stacktrace::detail::to_bt(rhs.impl_)
    ;
}

}}

#endif // BOOST_STACKTRACE_SRC_STACKTRACE_IPP
