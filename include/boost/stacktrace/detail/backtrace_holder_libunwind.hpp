// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_LIBUNWIND_HPP
#define BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_LIBUNWIND_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/demangle.hpp>
#include <cstring>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/make_shared.hpp>
#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <algorithm>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    std::size_t frames_count;
    boost::shared_ptr<std::string[]> frames;

    inline std::size_t size() const BOOST_NOEXCEPT {
        return frames_count;
    }

    inline const void* get_address(std::size_t frame) const BOOST_NOEXCEPT {
        return &frames[frame];
    }

    inline std::string get_frame(std::size_t frame) const {
        if (frame < frames_count) {
            return frames[frame];
        } else {
            return std::string();
        }
    }

    static inline std::string get_frame_impl(unw_cursor_t& cursor) {
        std::string res;
        unw_word_t offp;
        char data[256];
        const int ret = unw_get_proc_name (&cursor, data, sizeof(data) / sizeof(char), &offp);

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

    inline bool operator< (const backtrace_holder& rhs) const BOOST_NOEXCEPT {
        if (frames_count != rhs.frames_count) {
            return frames_count < rhs.frames_count;
        } else if (frames.get() == rhs.frames.get()) {
            return false;
        }

        return std::lexicographical_compare(
            frames.get(), frames.get() + frames_count,
            rhs.frames.get(), rhs.frames.get() + rhs.frames_count
        );
    }

    inline bool operator==(const backtrace_holder& rhs) const BOOST_NOEXCEPT {
        if (frames_count != rhs.frames_count) {
            return false;
        } else if (frames.get() == rhs.frames.get()) {
            return true;
        }

        return std::equal(
            frames.get(), frames.get() + frames_count,
            rhs.frames.get()
        );
    }
};

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_LIBUNWIND_HPP
