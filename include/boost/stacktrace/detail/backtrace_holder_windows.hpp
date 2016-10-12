// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_WINDOWS_HPP
#define BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_WINDOWS_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <algorithm>

#include <windows.h>
#include "DbgHelp.h"
#include <WinBase.h>

#include <boost/detail/winapi/get_current_process.hpp>
#include <boost/detail/winapi/sym_from_addr.hpp>

#if !defined(BOOST_ALL_NO_LIB)
#   define BOOST_LIB_NAME Dbghelp
#   ifdef BOOST_STACKTRACE_DYN_LINK
#       define BOOST_DYN_LINK
#   endif
#   include <boost/config/auto_link.hpp>
#endif

namespace boost { namespace stacktrace { namespace detail {

struct symbol_info_with_stack {
    BOOST_STATIC_CONSTEXPR std::size_t max_name_length = MAX_SYM_NAME * sizeof(char);
    boost::detail::winapi::SYMBOL_INFO_ symbol;
    char name_part[max_name_length];
};

struct symbol_initialization_structure {
    boost::detail::winapi::HANDLE_ process;

    inline symbol_initialization_structure() BOOST_NOEXCEPT
        : process(boost::detail::winapi::GetCurrentProcess())
    {
        SymInitialize(process, 0, true);
    }

    inline ~symbol_initialization_structure() BOOST_NOEXCEPT {
        SymCleanup(process);
    }
};

struct backtrace_holder {
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;

    std::size_t frames_count;
    void* buffer[max_size];

    inline std::size_t size() const BOOST_NOEXCEPT {
        return frames_count;
    }

    inline std::string get_frame(std::size_t frame) const {
        std::string res;

        static symbol_initialization_structure symproc;

        if (frame >= frames_count) {
            return res;
        }

        symbol_info_with_stack s;
        s.symbol.MaxNameLen = symbol_info_with_stack::max_name_length;
        s.symbol.SizeOfStruct = sizeof(boost::detail::winapi::SYMBOL_INFO_);
        const bool sym_res = !!boost::detail::winapi::SymFromAddr(
            symproc.process, reinterpret_cast<boost::detail::winapi::ULONGLONG_>(buffer[frame]), 0, &s.symbol
        );
        if (sym_res) {
            res = s.symbol.Name;
        } else {
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

#endif // BOOST_STACKTRACE_DETAIL_BACKTRACE_HOLDER_WINDOWS_HPP
