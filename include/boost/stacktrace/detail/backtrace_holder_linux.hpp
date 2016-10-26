// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_LINUX_HPP
#define BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_LINUX_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/demangle.hpp>
#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <algorithm>

#include <dlfcn.h>
#include <execinfo.h>

namespace boost { namespace stacktrace { namespace detail {

struct backtrace_holder {
    std::size_t frames_count;
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;
    void* buffer[max_size];

    inline std::size_t size() const BOOST_NOEXCEPT {
        return frames_count;
    }

    inline const void* get_address(std::size_t frame) const BOOST_NOEXCEPT {
        return buffer[frame];
    }

    inline std::string get_frame(std::size_t frame) const {
        std::string res;
        if (frame >= frames_count) {
            return res;
        }

        Dl_info dli;
        if (!!dladdr(buffer[frame], &dli) && dli.dli_sname) {
            boost::core::scoped_demangled_name demangled(dli.dli_sname);
            if (demangled.get()) {
                res = demangled.get();
            } else {
                res = dli.dli_sname;
            }
        } else  {
            res = "?? at ";
            res += to_hex_array(buffer[frame]).data();
        }

        return res;
    }

    inline bool operator< (const backtrace_holder& rhs) const BOOST_NOEXCEPT {
        if (frames_count != rhs.frames_count) {
            return frames_count < rhs.frames_count;
        } else if (this == &rhs) {
            return false;
        }

        return std::lexicographical_compare(
            buffer, buffer + frames_count,
            rhs.buffer, rhs.buffer + rhs.frames_count
        );
    }

    inline bool operator==(const backtrace_holder& rhs) const BOOST_NOEXCEPT {
        if (frames_count != rhs.frames_count) {
            return false;
        } else if (this == &rhs) {
            return true;
        }

        return std::equal(
            buffer, buffer + frames_count,
            rhs.buffer
        );
    }
};

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_LINUX_HPP
