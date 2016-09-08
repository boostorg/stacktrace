// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <boost/stacktrace/detail/stacktrace_helpers.hpp>
#include <boost/core/no_exceptions_support.hpp>

#include <cstring>
#include <windows.h>
#include "DbgHelp.h"
#include <WinBase.h>

namespace boost { namespace stacktrace { namespace detail {

struct symbol_info_with_stack {
    SYMBOL_INFO symbol;
    char name_part[MAX_SYM_NAME * sizeof(TCHAR)];
};

struct backtrace_holder {
    BOOST_STATIC_CONSTEXPR std::size_t max_size = 100u;

    std::size_t frames_count;
    void* buffer[max_size];
    HANDLE process;

    inline backtrace_holder() BOOST_NOEXCEPT
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

    inline std::string get_frame(std::size_t frame) const BOOST_NOEXCEPT {
        std::string res;

        if (frame >= frames_count || !process) {
            return res;
        }

        symbol_info_with_stack s;
        s.symbol.MaxNameLen = f.name.size() - 1;
        s.symbol.SizeOfStruct = sizeof(SYMBOL_INFO);

        SymFromAddr(process, reinterpret_cast<DWORD64>(buffer[frame]), 0, &s.symbol);
        BOOST_TRY {
            res = s.symbol.Name;
        } BOOST_CATCH(...) {}
        BOOST_CATCH_END
        return res;
    }

};

}}} // namespace boost::stacktrace::detail

