// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_SAFE_DUMP_WIN_IPP
#define BOOST_STACKTRACE_DETAIL_SAFE_DUMP_WIN_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>

#include <boost/core/noncopyable.hpp>

#include <boost/detail/winapi/get_current_process.hpp>
#include <boost/detail/winapi/file_management.hpp>
#include <boost/detail/winapi/handles.hpp>
#include <boost/detail/winapi/access_rights.hpp>

namespace boost { namespace stacktrace { namespace detail {

std::size_t dump(void* fd, void** memory, std::size_t mem_size) BOOST_NOEXCEPT {
    if (!boost::detail::winapi::WriteFile(fd, memory, static_cast<boost::detail::winapi::DWORD_>(sizeof(void*) * mem_size), 0, 0)) {
        return 0;
    }

    return mem_size;
}

std::size_t dump(const char* file, void** memory, std::size_t mem_size) BOOST_NOEXCEPT {
    void* const fd = boost::detail::winapi::CreateFileA(
        file,
        boost::detail::winapi::GENERIC_WRITE_,
        0,
        0,
        boost::detail::winapi::CREATE_ALWAYS_,
        boost::detail::winapi::FILE_ATTRIBUTE_NORMAL_,
        0
    );

    if (fd == boost::detail::winapi::invalid_handle_value) {
        return 0;
    }

    const std::size_t size = boost::stacktrace::detail::dump(fd, memory, mem_size);
    boost::detail::winapi::CloseHandle(fd);
    return size;
}

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_SAFE_DUMP_WIN_IPP
