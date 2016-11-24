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
#include <boost/lexical_cast/try_lexical_convert.hpp>
#include <algorithm>

#include <dlfcn.h>
#include <execinfo.h>
#include <cstdio>



namespace boost { namespace stacktrace { namespace detail {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

struct backtrace_holder {
    std::size_t frames_count;
    void* buffer[];

    backtrace_holder() BOOST_NOEXCEPT {}

    backtrace_holder(const backtrace_holder& d) BOOST_NOEXCEPT 
        : frames_count(d.frames_count)
    {
        std::copy(d.buffer, d.buffer + frames_count, buffer);
    }
};

#pragma GCC diagnostic pop

class pipe_holder {
    FILE* p;

public:
    explicit pipe_holder(const char* command) BOOST_NOEXCEPT
        : p(popen(command,"r"))
    {}

    operator FILE*() const BOOST_NOEXCEPT {
        return p;
    }

    ~pipe_holder() BOOST_NOEXCEPT {
        if (p) {
            pclose(p);
        }
    }
};

static inline std::string addr2line(const char* command, void* addr) {
    std::string res;

    Dl_info dli;
    if (!!dladdr(addr, &dli) && dli.dli_fname) {
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
    pipe_holder p(
        (command + res + " " + to_hex_array(addr).data()).c_str()
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

    // Trimming
    while (!res.empty() && (res[res.size() - 1] == '\n' || res[res.size() - 1] == '\r')) {
        res.erase(res.size() - 1);
    }

    return res;
}

static inline std::string try_demangle(const char* mangled) {
    std::string res;

    boost::core::scoped_demangled_name demangled(mangled);
    if (demangled.get()) {
        res = demangled.get();
    } else {
        res = mangled;
    }

    return res;
}

backend::backend(void* memory, std::size_t size, std::size_t& hash_code) BOOST_NOEXCEPT
    : data_(memory)
{
    new (data_) backtrace_holder();
    impl().frames_count = 0;
    hash_code = 0;

    // TODO: Not async signal safe. Use _Unwind_Backtrace, _Unwind_GetIP
    impl().frames_count = ::backtrace(impl().buffer, (size - sizeof(backtrace_holder)) / sizeof(void*));
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
        res = try_demangle(dli.dli_sname);
    } else {
        res = addr2line("addr2line -fe ", impl().buffer[frame]);
        res = res.substr(0, res.find_last_of('\n'));
        res = try_demangle(res.c_str());
    }

    return res;
}

const void* backend::get_address(std::size_t frame) const BOOST_NOEXCEPT {
    return impl().buffer[frame];
}

std::string backend::get_source_file(std::size_t frame) const {
    std::string res = addr2line("addr2line -e ", impl().buffer[frame]);
    res = res.substr(0, res.find_last_of(':'));
    return res;
}

std::size_t backend::get_source_line(std::size_t frame) const BOOST_NOEXCEPT {
    std::string res = addr2line("addr2line -e ", impl().buffer[frame]);
    const std::size_t last = res.find_last_of(':');
    if (last == std::string::npos) {
        return 0;
    }
    res = res.substr(last + 1);

    std::size_t line_num = 0;
    if (!boost::conversion::try_lexical_convert(res, line_num)) {
        return 0;
    }

    return line_num;
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
