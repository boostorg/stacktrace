// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_LINUX_HPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_LINUX_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/core/demangle.hpp>

#include <dlfcn.h>
#include <execinfo.h>

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    std::size_t frames_count;
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;
    void* buffer[max_size];

#ifdef BOOST_STACKTRACE_HEADER_ONLY
    BOOST_STATIC_CONSTEXPR std::size_t skip_frames = 0u;
#else
    BOOST_STATIC_CONSTEXPR std::size_t skip_frames = 2u;
#endif

    inline backtrace_holder() BOOST_NOEXCEPT {
        frames_count = ::backtrace(buffer, max_size);
    }

    inline std::size_t size() const BOOST_NOEXCEPT {
        return frames_count > skip_frames ? frames_count - skip_frames : 1u;
    }

    inline std::string get_frame(std::size_t frame) const {
        std::string res;
        frame += skip_frames;
        if (frame >= frames_count) {
            return res;
        }

        Dl_info dli;
        if (!dladdr(buffer[frame], &dli)) {
            return res;
        }

        if (dli.dli_sname) {
            boost::core::scoped_demangled_name demangled(dli.dli_sname);
            if (demangled.get()) {
                res = demangled.get();
            } else {
                res = dli.dli_sname;
            }
        }

        return res;
    }
};


}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_LINUX_HPP
