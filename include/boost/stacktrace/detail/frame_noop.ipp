// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_FRAME_NOOP_IPP
#define BOOST_STACKTRACE_DETAIL_FRAME_NOOP_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>

namespace boost { namespace stacktrace { namespace detail {

std::string to_string(const frame* /*frames*/, std::size_t /*count*/) {
    return std::string();
}

std::size_t this_thread_frames::collect(native_frame_ptr_t* /*out_frames*/, std::size_t /*max_frames_count*/, std::size_t /*skip*/) BOOST_NOEXCEPT {
    return 0;
}

#if defined(BOOST_WINDOWS)
std::size_t dump(void* /*fd*/, const native_frame_ptr_t* /*frames*/, std::size_t /*frames_count*/) BOOST_NOEXCEPT {
    return 0;
}
#else
std::size_t dump(int /*fd*/, const native_frame_ptr_t* /*frames*/, std::size_t /*frames_count*/) BOOST_NOEXCEPT {
    return 0;
}
#endif


std::size_t dump(const char* /*file*/, const native_frame_ptr_t* /*frames*/, std::size_t /*frames_count*/) BOOST_NOEXCEPT {
    return 0;
}

} // namespace detail

std::string frame::name() const {
    return std::string();
}

std::string frame::source_file() const {
    return std::string();
}

std::size_t frame::source_line() const {
    return 0;
}

std::string to_string(const frame& /*f*/) {
    return std::string();
}


}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_FRAME_NOOP_IPP
