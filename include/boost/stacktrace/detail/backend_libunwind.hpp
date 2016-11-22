// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKEND_LIBUNWIND_HPP
#define BOOST_STACKTRACE_DETAIL_BACKEND_LIBUNWIND_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/demangle.hpp>
#include <cstring>
#include <boost/functional/hash.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/make_shared.hpp>
#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <algorithm>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace boost { namespace stacktrace { namespace detail {

struct bt_pair {
    std::string name;
    const void* addr;
};

inline bool operator< (const bt_pair& lhs, const bt_pair& rhs) BOOST_NOEXCEPT {
    return lhs.addr < rhs.addr;
}

inline bool operator== (const bt_pair& lhs, const bt_pair& rhs) BOOST_NOEXCEPT {
    return lhs.addr == rhs.addr;
}

struct backtrace_holder {
    std::size_t frames_count;
    boost::shared_ptr<bt_pair[]> frames;
};

inline std::string get_frame_impl(unw_cursor_t& cursor) {
    std::string res;
    unw_word_t offp;
    char data[256];
    const int ret = unw_get_proc_name(&cursor, data, sizeof(data) / sizeof(char), &offp);

    if (ret == -UNW_ENOMEM) {
        res.resize(sizeof(data) * 2);
        do {
            const int ret2 = unw_get_proc_name(&cursor, &res[0], res.size(), &offp);
            if (ret2 == -UNW_ENOMEM) {
                res.resize(res.size() * 2);
            } else if (ret2 == 0) {
                break;
            } else {
                res = data;
                return res;
            }
        } while(1);
    } else if (ret == 0) {
        res = data;
    } else {
        return res;
    }

    boost::core::scoped_demangled_name demangled(res.data());
    if (demangled.get()) {
        res = demangled.get();
    } else {
        res.resize( std::strlen(res.data()) ); // Note: here res is \0 terminated, but size() not equal to strlen
    }

    res += " +";
    res += to_hex_array(offp).data();

    return res;
}



backend::backend(void* memory, std::size_t size, std::size_t& hash_code) BOOST_NOEXCEPT
    : data_(memory)
{
    new (data_) backtrace_holder();
    impl().frames_count = 0;
    hash_code = 0;

    unw_context_t uc;
    if (unw_getcontext(&uc) != 0) {
        return;
    }

    {   // Counting frames_count
        unw_cursor_t cursor;
        if (unw_init_local(&cursor, &uc) != 0) {
            return;
        }
        while (unw_step(&cursor) > 0) {
            ++ impl().frames_count;
        }
    }

    unw_cursor_t cursor;
    if (unw_init_local(&cursor, &uc) != 0) {
        impl().frames_count = 0;
        return;
    }

    BOOST_TRY {
        impl().frames = boost::make_shared<boost::stacktrace::detail::bt_pair[]>(impl().frames_count);
        std::size_t i = 0;
        while (unw_step(&cursor) > 0){
            impl().frames[i].name = get_frame_impl(cursor);
            unw_proc_info_t inf;
            const int res = unw_get_proc_info(&cursor, &inf);
            (void)res;
            impl().frames[i].addr = reinterpret_cast<void*>(inf.start_ip ? inf.start_ip : inf.gp);
            boost::hash_combine(hash_code, impl().frames[i].name);
            ++ i;
        }
    } BOOST_CATCH(...) {}
    BOOST_CATCH_END
}

std::string backend::get_name(std::size_t frame) const {
    if (frame < impl().frames_count) {
        return impl().frames[frame].name;
    } else {
        return std::string();
    }
}

const void* backend::get_address(std::size_t frame) const BOOST_NOEXCEPT {
    return impl().frames[frame].addr;
}

std::string backend::get_source_file(std::size_t /*frame*/) const {
    return std::string();
}

std::size_t backend::get_source_line(std::size_t /*frame*/) const BOOST_NOEXCEPT {
    return 0;
}

bool backend::operator< (const backend& rhs) const BOOST_NOEXCEPT {
    if (impl().frames_count != rhs.impl().frames_count) {
        return impl().frames_count < rhs.impl().frames_count;
    } else if (impl().frames.get() == rhs.impl().frames.get()) {
        return false;
    }

    return std::lexicographical_compare(
        impl().frames.get(), impl().frames.get() + impl().frames_count,
        rhs.impl().frames.get(), rhs.impl().frames.get() + rhs.impl().frames_count
    );
}

bool backend::operator==(const backend& rhs) const BOOST_NOEXCEPT {
    if (impl().frames_count != rhs.impl().frames_count) {
        return false;
    } else if (impl().frames.get() == rhs.impl().frames.get()) {
        return true;
    }

    return std::equal(
        impl().frames.get(), impl().frames.get() + impl().frames_count,
        rhs.impl().frames.get()
    );
}

}}} // namespace boost::stacktrace::detail

#include <boost/stacktrace/detail/backend_common.ipp>

#endif // BOOST_STACKTRACE_DETAIL_BACKEND_LIBUNWIND_HPP
