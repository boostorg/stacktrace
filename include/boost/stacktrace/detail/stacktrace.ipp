// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stacktrace.hpp>
#include <boost/static_assert.hpp>

#include <boost/predef/os/windows.h>

#if BOOST_OS_WINDOWS
#   include <boost/stacktrace/detail/stacktrace_windows.hpp>
//#elif defined(__has_include) && (!defined(__GNUC__) || __GNUC__ > 4)
//#   if __has_include(<libunwind.h>)
//#       include <boost/stacktrace/detail/stacktrace_libunwind.hpp>
//#   endif
#elif defined(BOOST_STACKTRACE_USE_LIBUNWIND)
#   include <boost/stacktrace/detail/stacktrace_libunwind.hpp>
#elif defined(BOOST_STACKTRACE_USE_BACKTRACE)
#   include <boost/stacktrace/detail/stacktrace_linux.hpp>
#else
#   error No suitable backtrace backend found
#endif

namespace boost { namespace stacktrace {

using boost::stacktrace::detail::backtrace_holder;

template <class T>
inline backtrace_holder& to_bt(T& data) BOOST_NOEXCEPT {
    return *reinterpret_cast<backtrace_holder*>(&data);
}

template <class T>
inline const backtrace_holder& to_bt(const T& data) BOOST_NOEXCEPT {
    return *reinterpret_cast<const backtrace_holder*>(&data);
}


stacktrace::stacktrace() BOOST_NOEXCEPT {
    new (&data_) backtrace_holder();
}

stacktrace::stacktrace(const stacktrace& bt) BOOST_NOEXCEPT {
    new (&data_) backtrace_holder(to_bt(bt.data_));
}

stacktrace& stacktrace::operator=(const stacktrace& bt) BOOST_NOEXCEPT {
    to_bt(data_) = to_bt(bt.data_);
    return *this;
}

stacktrace::~stacktrace() BOOST_NOEXCEPT {
    BOOST_STATIC_ASSERT_MSG(sizeof(data_) >= sizeof(backtrace_holder), "Too small storage for holding backtrace");
    to_bt(data_).~backtrace_holder();
}

std::size_t stacktrace::size() const BOOST_NOEXCEPT {
    return to_bt(data_).size();
}

stacktrace::frame_t stacktrace::operator[](std::size_t frame) const BOOST_NOEXCEPT {
    return to_bt(data_).get_frame(frame);
}


}}
