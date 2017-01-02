// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKEND_WINDOWS_HPP
#define BOOST_STACKTRACE_DETAIL_BACKEND_WINDOWS_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <windows.h>
#include "Dbgeng.h"

#include <boost/detail/winapi/get_current_process.hpp>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Dbgeng.lib")

namespace boost { namespace stacktrace { namespace detail {

class com_global_initer: boost::noncopyable {
public:
    com_global_initer() BOOST_NOEXCEPT {
        // We do not care about the result of the function call: any result is OK for us.
        CoInitializeEx(0, COINIT_MULTITHREADED);
    }
    ~com_global_initer() BOOST_NOEXCEPT {
        CoUninitialize();
    }
};

template <class T>
class com_holder: boost::noncopyable {
    T* holder_;

public:
    com_holder(const com_global_initer&) BOOST_NOEXCEPT
        : holder_(0)
    {}

    T* operator->() const BOOST_NOEXCEPT {
        return holder_;
    }

    void** to_void_ptr_ptr() BOOST_NOEXCEPT {
        return reinterpret_cast<void**>(&holder_);
    }

    bool is_inited() const BOOST_NOEXCEPT {
        return !!holder_;
    }

    ~com_holder() BOOST_NOEXCEPT {
        if (holder_) {
            holder_->Release();
        }
    }
};

inline bool try_init_com(com_holder<IDebugSymbols>& idebug, const com_global_initer& com) BOOST_NOEXCEPT {
    com_holder<IDebugClient> iclient(com);
    DebugCreate(__uuidof(IDebugClient), iclient.to_void_ptr_ptr());

    com_holder<IDebugControl> icontrol(com);
    iclient->QueryInterface(__uuidof(IDebugControl), icontrol.to_void_ptr_ptr());

    const bool res1 = (S_OK == iclient->AttachProcess(
        0,
        GetCurrentProcessId(),
        DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND)
    );
    if (!res1) {
        return false;
    }

    const bool res2 = (S_OK == icontrol->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE));
    if (!res2) {
        return false;
    }

    const bool res = (S_OK == iclient->QueryInterface(__uuidof(IDebugSymbols), idebug.to_void_ptr_ptr()));
    if (!res) {
        return false;
    }

    return true;
}




std::size_t backend::collect(void** memory, std::size_t size) BOOST_NOEXCEPT {
    return CaptureStackBackTrace(
        0,
        static_cast<boost::detail::winapi::ULONG_>(size),
        memory,
        0
    );
}

inline std::string get_name_impl(const com_holder<IDebugSymbols>& idebug, const void* addr) {
    std::string result;
    const ULONG64 offset = reinterpret_cast<ULONG64>(addr);

    char name[256];
    name[0] = '\0';
    ULONG size = 0;
    bool res = (S_OK == idebug->GetNameByOffset(
        offset,
        name,
        sizeof(name),
        &size,
        0
    ));

    if (!res && size != 0) {
        result.resize(size);
        res = (S_OK == idebug->GetNameByOffset(
            offset,
            &result[0],
            static_cast<ULONG>(result.size()),
            &size,
            0
        ));
    } else if (res) {
        result = name;
    }

    if (!res) {
        result.clear();
    }

    return result;
}


std::string backend::get_name(const void* addr) {
    com_global_initer com_guard;
    com_holder<IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }
    
    return boost::stacktrace::detail::get_name_impl(idebug, addr);
}


inline std::pair<std::string, std::size_t> get_source_file_line_impl(const com_holder<IDebugSymbols>& idebug, const void* addr) {
    std::pair<std::string, std::size_t> result;
    const ULONG64 offset = reinterpret_cast<ULONG64>(addr);

    char name[256];
    name[0] = 0;
    ULONG size = 0;
    ULONG line_num = 0;
    bool res = (S_OK == idebug->GetLineByOffset(
        offset,
        &line_num,
        name,
        sizeof(name),
        &size,
        0
    ));

    if (res) {
        result.first = name;
        result.second = line_num;
        return result;
    }

    if (!res && size == 0) {
        return result;
    }

    result.first.resize(size);
    res = (S_OK == idebug->GetLineByOffset(
        offset,
        &line_num,
        &result.first[0],
        static_cast<ULONG>(result.first.size()),
        &size,
        0
    ));
    result.second = line_num;

    if (!res) {
        result.first.clear();
        result.second = 0;
    }

    return result;
}


std::string backend::get_source_file(const void* addr) {
    com_global_initer com_guard;
    com_holder<IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }
    return boost::stacktrace::detail::get_source_file_line_impl(idebug, addr).first;
}

std::size_t backend::get_source_line(const void* addr) {
    ULONG line_num = 0;

    com_global_initer com_guard;
    com_holder<IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return 0;
    }

    const bool is_ok = (S_OK == idebug->GetLineByOffset(
        reinterpret_cast<ULONG64>(addr),
        &line_num,
        0,
        0,
        0,
        0
    ));

    return (is_ok ? line_num : 0);
}


std::string backend::to_string(const void* addr) {
    com_global_initer com_guard;
    com_holder<IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }

    std::pair<std::string, std::size_t> file_line
        = boost::stacktrace::detail::get_source_file_line_impl(idebug, addr);

    return boost::stacktrace::detail::get_name_impl(idebug, addr)
        + " at "
        + file_line.first
        + ':'
        + boost::lexical_cast<std::string>(file_line.second)
    ;
}

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_BACKEND_LINUX_HPP
