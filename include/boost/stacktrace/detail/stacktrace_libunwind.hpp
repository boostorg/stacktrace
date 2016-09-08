// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_LIBUNWIND_HPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_LIBUNWIND_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/core/demangle.hpp>
#include <cstring>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/make_shared.hpp>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    BOOST_STATIC_CONSTEXPR std::size_t skip_frames = 1u;
    std::size_t frames_count;
    boost::shared_ptr<std::string[]> frames;

    backtrace_holder() BOOST_NOEXCEPT
        : frames_count(0)
    {
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
                ++ frames_count;
            }
            if (frames_count <= skip_frames) {
                frames_count = 0;
                return;
            }
            frames_count -= skip_frames;
        }

        unw_cursor_t cursor;
        if (unw_init_local(&cursor, &uc) != 0) {
            frames_count = 0;
            return;
        }

        for (std::size_t i = 0; i < skip_frames; ++i) {
            if (unw_step(&cursor) <= 0) {
                frames_count = 0;
                return;
            }
        }

        BOOST_TRY {
            frames = boost::make_shared<std::string[]>(frames_count);
            std::size_t i = 0;
            while (unw_step(&cursor) > 0){
                frames[i] = get_frame_impl(cursor);
                ++ i;
            }
        } BOOST_CATCH(...) {}
        BOOST_CATCH_END
    }

    std::size_t size() const BOOST_NOEXCEPT {
        return frames_count;
    }

    std::string get_frame(std::size_t frame) const {
        if (frame < frames_count) {
            return frames[frame];
        } else {
            return std::string();
        }
    }

    static std::string get_frame_impl(unw_cursor_t& cursor) {
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

        return res;
    }
};

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_LIBUNWIND_HPP
