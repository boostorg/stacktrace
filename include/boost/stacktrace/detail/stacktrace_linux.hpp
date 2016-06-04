// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/core/demangle.hpp>

#include <dlfcn.h>
#include <execinfo.h>
#include <cstddef>
#include <cstring>

namespace boost { namespace stacktrace { namespace detail {

typedef boost::stacktrace::stacktrace::frame_t frame_t;

struct backtrace_holder {
    std::size_t frames_count;
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;
    void* buffer[max_size];

    backtrace_holder() BOOST_NOEXCEPT {
        frames_count = ::backtrace(buffer, max_size);
    }

    std::size_t size() const BOOST_NOEXCEPT {
        return frames_count;
    }

    frame_t get_frame(std::size_t frame) const BOOST_NOEXCEPT {
        frame_t name = {"??"};
        name.back() = '\0';
        if (frame >= frames_count) {
            return name;
        }

        Dl_info dli;
        if (dladdr(buffer[frame], &dli) && dli.dli_sname) {
            boost::core::scoped_demangled_name demangled(dli.dli_sname);
            if (demangled.get()) {
                std::strncpy(name.data(), demangled.get(), name.size() - 1);
            } else {
                std::strncpy(name.data(), dli.dli_sname, name.size() - 1);
            }
        } else {
            std::strncpy(name.data(), to_hex(reinterpret_cast<std::ptrdiff_t>(buffer[frame])).data(), name.size() - 1);
        }

        return name;
    }

};


}}} // namespace boost::stacktrace::detail
