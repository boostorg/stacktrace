// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKEND_COMMON_IPP
#define BOOST_STACKTRACE_DETAIL_BACKEND_COMMON_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

namespace boost { namespace stacktrace { namespace detail {

backend::backend(const backend& b, void* memory) BOOST_NOEXCEPT
    : data_(memory)
{
    new(data_) backtrace_holder(
        b.impl()
    );
}

backend::~backend() BOOST_NOEXCEPT {
    reinterpret_cast<backtrace_holder*>(data_)->~backtrace_holder();
}

std::size_t backend::size() const BOOST_NOEXCEPT {
    return impl().frames_count;
}

}}}

#endif // BOOST_STACKTRACE_DETAIL_BACKEND_COMMON_IPP
