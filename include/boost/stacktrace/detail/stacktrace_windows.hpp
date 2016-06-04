// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/assert.hpp>

#include <cstring>
#include <windows.h>
#include "DbgHelp.h"
#include <WinBase.h>
#pragma comment(lib, "Dbghelp.lib")

namespace boost { namespace stacktrace { namespace detail {

typedef boost::stacktrace::stacktrace::frame_t frame_t;

struct symbol_info_with_stack {
    SYMBOL_INFO symbol;
    char name_part[frame_t::static_size - 1];
};

struct backtrace_holder {
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;

    std::size_t frames_count;
    void* buffer[max_size];
    HANDLE process;

    backtrace_holder() BOOST_NOEXCEPT
        : frames_count(1)
        , process(GetCurrentProcess())
    {
        BOOST_VERIFY(process);

        const bool inited = !!SymInitialize(process, 0, true);
        BOOST_VERIFY(inited);
        if (inited) {
            frames_count = CaptureStackBackTrace(0, max_size, buffer, 0);
        }
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

        symbol_info_with_stack s;
        s.symbol.MaxNameLen = name.size() - 1;
        s.symbol.SizeOfStruct = sizeof(SYMBOL_INFO);

        SymFromAddr(process, (DWORD64)(buffer[frame]), 0, &s.symbol);
        std::strncpy(name.data(), s.symbol.Name, name.size() - 1);
        std::strncat(name.data(), " ", name.size() - 1);
        std::strncat(name.data(), to_hex(s.symbol.Address).data(), name.size() - 1);

        return name;
    }

};

}}} // namespace boost::stacktrace::detail

