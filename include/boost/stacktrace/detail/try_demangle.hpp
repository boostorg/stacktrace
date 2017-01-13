// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_TRY_DEMANGLE_HPP
#define BOOST_STACKTRACE_DETAIL_TRY_DEMANGLE_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/demangle.hpp>

namespace boost { namespace stacktrace { namespace detail {

inline std::string try_demangle(const char* mangled) {
    std::string res;

    boost::core::scoped_demangled_name demangled(mangled);
    if (demangled.get()) {
        res = demangled.get();
    } else {
        res = mangled;
    }

    return res;
}


}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_TRY_DEMANGLE_HPP
