// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_FRAME_UNWIND_IPP
#define BOOST_STACKTRACE_DETAIL_FRAME_UNWIND_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>

#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <boost/stacktrace/detail/try_demangle.hpp>
#include <boost/lexical_cast.hpp>

#include <unwind.h>

#include <dlfcn.h>
#include <execinfo.h>
#include <cstdio>

#ifdef BOOST_STACKTRACE_USE_BACKTRACE
#   include <boost/stacktrace/detail/libbacktrace_impls.hpp>
#elif defined(BOOST_STACKTRACE_USE_ADDR2LINE)
#   include <boost/stacktrace/detail/addr2line_impls.hpp>
#else
#   include <boost/stacktrace/detail/unwind_base_impls.hpp>
#endif

namespace boost { namespace stacktrace { namespace detail {

struct unwind_state {
    void** current;
    void** end;
};

inline _Unwind_Reason_Code unwind_callback(::_Unwind_Context* context, void* arg) {
    unwind_state* state = static_cast<unwind_state*>(arg);
    *state->current = reinterpret_cast<void*>(
        ::_Unwind_GetIP(context)
    );

    ++state->current;
    if (!*(state->current - 1) || state->current == state->end) {
        return ::_URC_END_OF_STACK;
    }
    return ::_URC_NO_REASON;
}

template <class Base>
class to_string_impl_base: private Base {
public:
    std::string operator()(const void* addr) {
        Base::res.clear();
        Base::prepare_function_name(addr);
        if (Base::res.empty()) {
            Base::res = to_hex_array(addr).data();
        }

        if (Base::prepare_source_location(addr)) {
            return Base::res;
        }

        ::Dl_info dli;
        if (!!::dladdr(addr, &dli) && dli.dli_sname) {
            Base::res += " in ";
            Base::res += dli.dli_fname;
        }

        return Base::res;
    }
};

std::string to_string(const frame* frames, std::size_t size) {
    std::string res;
    res.reserve(64 * size);

    to_string_impl impl;

    for (std::size_t i = 0; i < size; ++i) {
        if (i < 10) {
            res += ' ';
        }
        res += boost::lexical_cast<boost::array<char, 40> >(i).data();
        res += '#';
        res += ' ';
        res += impl(frames[i].address());
        res += '\n';
    }

    return res;
}

} // namespace detail


std::string frame::name() const {
    ::Dl_info dli;
    const bool dl_ok = !!::dladdr(addr_, &dli);
    if (dl_ok && dli.dli_sname) {
        return boost::stacktrace::detail::try_demangle(dli.dli_sname);
    }

    return boost::stacktrace::detail::name_impl(addr_);
}

std::string to_string(const frame& f) {
    boost::stacktrace::detail::to_string_impl impl;
    return impl(f.address());
}


std::size_t this_thread_frames::collect(void** memory, std::size_t size) BOOST_NOEXCEPT {
    std::size_t frames_count = 0;
    if (!size) {
        return frames_count;
    }

    boost::stacktrace::detail::unwind_state state = { memory, memory + size };
    ::_Unwind_Backtrace(&boost::stacktrace::detail::unwind_callback, &state);
    frames_count = state.current - memory;

    if (memory[frames_count - 1] == 0) {
        -- frames_count;
    }

    return frames_count;
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_FRAME_UNWIND_IPP
