// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_NOOP_HPP
#define BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_NOOP_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif


namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    inline std::size_t size() const BOOST_NOEXCEPT {
        return 0u;
    }

    inline std::string get_frame(std::size_t /*frame*/) const {
        return std::string();
    }
    
    inline bool operator< (const backtrace_holder& rhs) const BOOST_NOEXCEPT {
        return false;
    }

    inline bool operator==(const backtrace_holder& rhs) const BOOST_NOEXCEPT {
        return true;
    }
};

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_NOOP_HPP
