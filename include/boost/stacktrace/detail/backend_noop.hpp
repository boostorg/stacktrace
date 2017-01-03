// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKEND_NOOP_HPP
#define BOOST_STACKTRACE_DETAIL_BACKEND_NOOP_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

namespace boost { namespace stacktrace { namespace detail {


std::size_t backend::collect(void** /*memory*/, std::size_t /*size*/) BOOST_NOEXCEPT {
    return 0;
}

std::string backend::to_string(const void* /*addr*/) {
    return std::string();
}

} // namespace detail

std::string frame::name() const {
    return std::string();
}

std::string frame::source_file() const {
    return std::string();
}

std::size_t frame::source_line() const {
    return 0;
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_BACKEND_LIBUNWIND_HPP
