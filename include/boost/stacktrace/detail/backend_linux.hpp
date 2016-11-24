// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKEND_LINUX_HPP
#define BOOST_STACKTRACE_DETAIL_BACKEND_LINUX_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/demangle.hpp>
#include <boost/functional/hash.hpp>
#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <algorithm>

#include <dlfcn.h>
#include <execinfo.h>
#include <cstdio>

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    std::size_t frames_count;
    void* buffer[1];

    backtrace_holder() BOOST_NOEXCEPT {}

    backtrace_holder(const backtrace_holder& d) BOOST_NOEXCEPT 
        : frames_count(d.frames_count)
    {
        std::copy(d.buffer, d.buffer + frames_count, buffer);
    }
};

backend::backend(void* memory, std::size_t size, std::size_t& hash_code) BOOST_NOEXCEPT
    : data_(memory)
{
    new (data_) backtrace_holder();
    impl().frames_count = 0;
    hash_code = 0;

    // TODO: Not async signal safe. Use _Unwind_Backtrace, _Unwind_GetIP
    impl().frames_count = ::backtrace(impl().buffer, 1 + (size - sizeof(backtrace_holder)) / sizeof(void*));
    if (impl().buffer[impl().frames_count - 1] == 0) {
        -- impl().frames_count;
    }

    hash_code = boost::hash_range(impl().buffer, impl().buffer + impl().frames_count);
}

std::string backend::get_name(std::size_t frame) const {
    std::string res;
    if (frame >= impl().frames_count) {
        return res;
    }

    Dl_info dli;
    if (!!dladdr(impl().buffer[frame], &dli) && dli.dli_sname) {
        boost::core::scoped_demangled_name demangled(dli.dli_sname);
        if (demangled.get()) {
            res = demangled.get();
        } else {
            res = dli.dli_sname;
        }
    } else  {
        res = "?? at ";
        res += to_hex_array(impl().buffer[frame]).data();
    }

    return res;
}

const void* backend::get_address(std::size_t frame) const BOOST_NOEXCEPT {
    return impl().buffer[frame];
}

std::string backend::get_source_file(std::size_t frame) const {
    std::string res;

/*
    Dl_info dli;
    if (!!dladdr(impl().buffer[frame], &dli) && dli.dli_fname) {
        res = dli.dli_fname;
    } else {
        res.resize(16);
        int rlin_size = readlink("/proc/self/exe", &res[0], res.size() - 1);
        while (rlin_size == static_cast<int>(res.size() - 1)) {
            res.resize(res.size() * 4);
            rlin_size = readlink("/proc/self/exe", &res[0], res.size() - 1);
        }
        if (rlin_size == -1) {
            res.clear();
            return res;
        }
        res.resize(rlin_size);
    }

    // TODO: redirect STDERR
    FILE* p = popen(
        ("addr2line -e " + res + " " + to_hex_array(impl().buffer[frame]).data()).c_str(),
        "r"
    );
    res.clear();

    if (!p) {
        return res;
    }

    char data[32];
    while (!std::feof(p)) {
        if (std::fgets(data, sizeof(data), p)) {
            res += data;
        } else {
            break;
        }
    }
    pclose(p); // TODO: RAII

    res = res.substr(0, res.find_last_of(':'));
*/
    return res;
}

std::size_t backend::get_source_line(std::size_t /*frame*/) const BOOST_NOEXCEPT {
    return 0;
}

bool backend::operator< (const backend& rhs) const BOOST_NOEXCEPT {
    if (impl().frames_count != rhs.impl().frames_count) {
        return impl().frames_count < rhs.impl().frames_count;
    } else if (this == &rhs) {
        return false;
    }

    return std::lexicographical_compare(
        impl().buffer, impl().buffer + impl().frames_count,
        rhs.impl().buffer, rhs.impl().buffer + rhs.impl().frames_count
    );
}

bool backend::operator==(const backend& rhs) const BOOST_NOEXCEPT {
    if (impl().frames_count != rhs.impl().frames_count) {
        return false;
    } else if (this == &rhs) {
        return true;
    }

    return std::equal(
        impl().buffer, impl().buffer + impl().frames_count,
        rhs.impl().buffer
    );
}

}}} // namespace boost::stacktrace::detail

#include <boost/stacktrace/detail/backend_common.ipp>

#endif // BOOST_STACKTRACE_DETAIL_BACKEND_LINUX_HPP
