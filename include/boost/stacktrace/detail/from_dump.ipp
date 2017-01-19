// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_FROM_DUMP_IPP
#define BOOST_STACKTRACE_DETAIL_FROM_DUMP_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>
#include <fstream>

namespace boost { namespace stacktrace { namespace detail {

std::size_t from_dump(const char* filename, void** frames) {
    std::ifstream ifs(filename, std::ifstream::ate | std::ifstream::binary);
    std::size_t size = static_cast<std::size_t>(ifs.tellg()) / sizeof(void*);
    if (size > boost::stacktrace::detail::max_frames_dump) {
        size = boost::stacktrace::detail::max_frames_dump;
    }

    ifs.seekg(0);
    ifs.read(reinterpret_cast<char*>(frames), size * sizeof(void*));

    return size;
}

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_FROM_DUMP_IPP
