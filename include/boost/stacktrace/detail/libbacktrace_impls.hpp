// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_LIBBACKTRACE_IMPLS_HPP
#define BOOST_STACKTRACE_DETAIL_LIBBACKTRACE_IMPLS_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <boost/stacktrace/detail/try_demangle.hpp>
#include <boost/lexical_cast.hpp>

#include <backtrace.h>

namespace boost { namespace stacktrace { namespace detail {


struct pc_data {
    std::string* function;
    std::string* filename;
    std::size_t line;
};

inline int libbacktrace_full_callback(void *data, uintptr_t /*pc*/, const char *filename, int lineno, const char *function) {
    pc_data& d = *static_cast<pc_data*>(data);
    if (d.filename && filename) {
        *d.filename = filename;
    }
    if (d.function && function) {
        *d.function = function;
    }
    d.line = lineno;
    return 0;
}

struct to_string_using_backtrace {
    std::string res;
    ::backtrace_state* state;
    std::string filename;
    std::size_t line;

    void prepare_function_name(const void* addr) {
        boost::stacktrace::detail::pc_data data = {&res, &filename, 0};
        ::backtrace_pcinfo(state, reinterpret_cast<uintptr_t>(addr), boost::stacktrace::detail::libbacktrace_full_callback, 0, &data);
        line = data.line;
    }

    bool prepare_source_location(const void* /*addr*/) {
        if (filename.empty() || !line) {
            return false;
        }

        res += " at ";
        res += filename;
        res += ':';
        res += boost::lexical_cast<boost::array<char, 40> >(line).data();
        return true;
    }

    to_string_using_backtrace() BOOST_NOEXCEPT {
        state = ::backtrace_create_state(
            0, 0, 0, 0
        );
    }
};

template <class Base> class to_string_impl_base;
typedef to_string_impl_base<to_string_using_backtrace> to_string_impl;

inline std::string name_impl(const void* addr) {
    std::string res;

    ::backtrace_state* state = ::backtrace_create_state(
        0, 0, 0, 0
    );

    boost::stacktrace::detail::pc_data data = {&res, 0, 0};
    ::backtrace_pcinfo(state, reinterpret_cast<uintptr_t>(addr), boost::stacktrace::detail::libbacktrace_full_callback, 0, &data);
    if (!res.empty()) {
        res = boost::stacktrace::detail::try_demangle(res.c_str());
    }

    return res;
}

} // namespace detail

std::string frame::source_file() const {
    std::string res;

    ::backtrace_state* state = ::backtrace_create_state(
        0, 0, 0, 0
    );

    boost::stacktrace::detail::pc_data data = {0, &res, 0};
    ::backtrace_pcinfo(state, reinterpret_cast<uintptr_t>(addr_), boost::stacktrace::detail::libbacktrace_full_callback, 0, &data);

    return res;
}

std::size_t frame::source_line() const {
    ::backtrace_state* state = ::backtrace_create_state(
        0, 0, 0, 0
    );

    boost::stacktrace::detail::pc_data data = {0, 0, 0};
    ::backtrace_pcinfo(state, reinterpret_cast<uintptr_t>(addr_), boost::stacktrace::detail::libbacktrace_full_callback, 0, &data);


    return data.line;
}


}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_LIBBACKTRACE_IMPLS_HPP
