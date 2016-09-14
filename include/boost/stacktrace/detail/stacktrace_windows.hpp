// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_STACKTRACE_WINDOWS_HPP
#define BOOST_STACKTRACE_DETAIL_STACKTRACE_WINDOWS_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>

#include <windows.h>
#include "DbgHelp.h"
#include <WinBase.h>

#if !defined(BOOST_ALL_NO_LIB)
#   define BOOST_LIB_NAME Dbghelp
#   ifdef BOOST_STACKTRACE_DYN_LINK
#       define BOOST_DYN_LINK
#   endif
#   include <boost/config/auto_link.hpp>
#endif

namespace boost { namespace stacktrace { namespace detail {

struct symbol_info_with_stack {
    BOOST_STATIC_CONSTEXPR  std::size_t max_name_length = MAX_SYM_NAME * sizeof(char);
    SYMBOL_INFO symbol;
    char name_part[max_name_length];
};

struct backtrace_holder {
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;

    std::size_t frames_count;
    void* buffer[max_size];
    HANDLE process;

    BOOST_FORCEINLINE backtrace_holder() BOOST_NOEXCEPT
        : frames_count(0)
        , process(GetCurrentProcess())
    {
        const bool inited = !!SymInitialize(process, 0, true);
        if (inited) {
            frames_count = CaptureStackBackTrace(0, max_size, buffer, 0);
        } else {
            process = 0;
        }
    }

    inline std::size_t size() const BOOST_NOEXCEPT {
        return frames_count;
    }

    inline std::string get_frame(std::size_t frame) const {
        std::string res;

        if (frame >= frames_count || !process) {
            return res;
        }

        symbol_info_with_stack s;
        s.symbol.MaxNameLen = symbol_info_with_stack::max_name_length;
        s.symbol.SizeOfStruct = sizeof(SYMBOL_INFO);
        SymFromAddr(process, reinterpret_cast<DWORD64>(buffer[frame]), 0, &s.symbol);
        res = s.symbol.Name;
        return res;
    }

};

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_STACKTRACE_WINDOWS_HPP
