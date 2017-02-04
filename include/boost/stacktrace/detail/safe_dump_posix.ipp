// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_SAFE_DUMP_POSIX_IPP
#define BOOST_STACKTRACE_DETAIL_SAFE_DUMP_POSIX_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>

#include <unistd.h>     // ::write
#include <fcntl.h>      // ::open


namespace boost { namespace stacktrace { namespace detail {

std::size_t dump(int fd, void** memory, std::size_t size) BOOST_NOEXCEPT {
    // We do not retry, because this function must be typically called from signal handler so it's:
    //  * to scary to continue in case of EINTR
    //  * EAGAIN or EWOULDBLOCK may occur only in case of O_NONBLOCK is set for fd,
    // so it seems that user does not want to block
    if (::write(fd, memory, sizeof(void*) * size) == -1) {
        return 0;
    }

    return size;
}

std::size_t dump(const char* file, void** memory, std::size_t mem_size) BOOST_NOEXCEPT {
    const int fd = ::open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IFREG | S_IWUSR | S_IRUSR);
    if (fd == -1) {
        return 0;
    }

    const std::size_t size = boost::stacktrace::detail::dump(fd, memory, mem_size);
    ::close(fd);
    return size;
}

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_SAFE_DUMP_POSIX_IPP
