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

#if defined(BOOST_STACKTRACE_USE_NOOP)
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

stacktrace::stacktrace(const stacktrace& bt) BOOST_NOEXCEPT
    : impl_(bt.impl_)
    , hash_code_(bt.hash_code_)
{}

stacktrace& stacktrace::operator=(const stacktrace& bt) BOOST_NOEXCEPT {
    impl_ = bt.impl_;
    hash_code_ = bt.hash_code_;
    return *this;
}

stacktrace::~stacktrace() BOOST_NOEXCEPT {}

std::size_t stacktrace::size() const BOOST_NOEXCEPT {
    return impl_.size();
}

std::string stacktrace::get_name(std::size_t frame) const {
    return impl_.get_frame(frame);
}

const void* stacktrace::get_address(std::size_t frame) const BOOST_NOEXCEPT {
    return impl_.get_address(frame);
}

std::string stacktrace::get_source_file(std::size_t frame) const {
    return impl_.get_source_file(frame);
}

std::size_t stacktrace::get_source_line(std::size_t frame) const BOOST_NOEXCEPT {
    return impl_.get_source_line(frame);
}

bool stacktrace::operator< (const stacktrace& rhs) const BOOST_NOEXCEPT {
    return hash_code_ < rhs.hash_code_ || (hash_code_ == rhs.hash_code_ && impl_ < rhs.impl_);
}

bool stacktrace::operator==(const stacktrace& rhs) const BOOST_NOEXCEPT {
    return hash_code_ == rhs.hash_code_ && impl_ == rhs.impl_;
}

}}

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_IPP
