// Copyright Antony Polukhin, 2023-2024.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_CURRENT_EXCEPTION_HPP
#define BOOST_STACKTRACE_CURRENT_EXCEPTION_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/stacktrace.hpp>

namespace boost { namespace stacktrace {

namespace impl {

#if defined(__GNUC__) && defined(__ELF__)

BOOST_NOINLINE BOOST_SYMBOL_VISIBLE __attribute__((weak))
const char* current_exception_stacktrace() noexcept;

#endif

} // namespace impl


inline stacktrace current_exception_stacktrace() noexcept {
    // Matches the constant from implementation
    constexpr std::size_t kStacktraceDumpSize = 4096;

    const char* trace = nullptr;
#if defined(__GNUC__) && defined(__ELF__)
    if (impl::current_exception_stacktrace) {
        trace = impl::current_exception_stacktrace();
    }
#endif

    if (trace) {
       return stacktrace::from_dump(trace, kStacktraceDumpSize);
    }
    return stacktrace{0, 0};
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_CURRENT_EXCEPTION_HPP
