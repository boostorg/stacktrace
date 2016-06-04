// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/core/demangle.hpp>
#include <cstring>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace boost { namespace stacktrace { namespace detail {

typedef boost::stacktrace::stacktrace::frame_t frame_t;

struct backtrace_holder {
    mutable unw_context_t uc;

    backtrace_holder() BOOST_NOEXCEPT {
        BOOST_VERIFY(unw_getcontext(&uc) == 0);
    }

    std::size_t size() const BOOST_NOEXCEPT {
        std::size_t frames = 0;
        unw_cursor_t cursor;
        BOOST_VERIFY(unw_init_local(&cursor, &uc) >= 0);
        while (unw_step(&cursor) > 0) ++frames;

        return frames - 1;
    }

    frame_t get_frame(std::size_t frame) const BOOST_NOEXCEPT {
        frame_t name = {"??"};
        name.back() = '\0';
        unw_cursor_t cursor;
        unw_word_t offp;

        const int ret_init = unw_init_local(&cursor, &uc);
        BOOST_VERIFY(ret_init == 0);
        if (ret_init == 0 && unw_step(&cursor) == 0) {
            return name;
        }

        while (frame) {
            if (unw_step(&cursor) == 0) {
                return name;
            }

            --frame;
        }

        const int ret = unw_get_proc_name (&cursor, name.data(), sizeof(name), &offp);
        if (ret == 0 && name[0]) {
            boost::core::scoped_demangled_name demangled(name.data());
            if (demangled.get()) {
                std::strncpy(name.data(), demangled.get(), name.size() - 1);
            }

            std::strncat(name.data(), " +", name.size() - 1);
            std::strncat(name.data(), to_hex(offp).data(), name.size() - 1);
        } else {
            unw_proc_info_t pi;
            const int new_ret = unw_get_proc_info(&cursor, &pi);
            if (new_ret == 0 && pi.start_ip) {
                std::strncpy(name.data(), to_hex(pi.start_ip).data(), name.size() - 1);
            } else {
                std::strncpy(name.data(), "??", name.size() - 1);
            }
        }

        return name;
    }
};

}}} // namespace boost::stacktrace::detail

