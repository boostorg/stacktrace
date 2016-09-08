// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/core/demangle.hpp>
#include <cstring>
#include <boost/core/no_exceptions_support.hpp>
#include <vector>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    BOOST_STATIC_CONSTEXPR std::size_t skip_frames = 1u;
    std::vector<std::string> frames;

    backtrace_holder() BOOST_NOEXCEPT {
        unw_context_t uc;
        if (unw_getcontext(&uc) != 0) {
            return;
        }

        unw_cursor_t cursor;
        if (unw_init_local(&cursor, &uc) != 0) {
            return;
        }

        for (std::size_t i = 0; i < skip_frames; ++i) {
            if (unw_step(&cursor) <= 0) {
                return;
            }
        }

        BOOST_TRY {
            while (unw_step(&cursor) > 0){
                frames.push_back(get_frame_impl(cursor));
            }
        } BOOST_CATCH(...) {}
        BOOST_CATCH_END
    }

    std::size_t size() const BOOST_NOEXCEPT {
        return frames.size();
    }

    std::string get_frame(std::size_t frame) const {
        return frames[frame];
    }

    static std::string get_frame_impl(unw_cursor_t& cursor) {
        std::string res;
        unw_word_t offp;
        char data[128];
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
