// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/array.hpp>
BOOST_NOINLINE void oops(std::size_t i);
BOOST_NOINLINE void foo(int i);
BOOST_NOINLINE void bar(int i);

BOOST_NOINLINE void oops(std::size_t i) {
    // std::terminate();
    // void (*p)() = 0; p();

    boost::array<int, 5> a = {{0, 1, 2, 3, 4}};
    foo(a[i]);
}

BOOST_NOINLINE void bar(int i) {
    boost::array<int, 5> a = {{0, 1, 2, 3, 4}};
    if (i < 5) {
        if (i >= 0) {
            foo(a[i]);
        } else {
            oops(i);
        }
    }
}

BOOST_NOINLINE void foo(int i) {
    bar(--i);
}

/*
../../../boost/array.hpp:123: T& boost::array<T, N>::operator[](boost::array<T, N>::size_type) [with T = int; long unsigned int N = 5ul; boost::array<T, N>::reference = int&; boost::array<T, N>::size_type = long unsigned int]: Assertion `(i < N)&&("out of range")' failed.
Aborted (core dumped)
*/

/*
Expression 'i < N' is false in function 'T& boost::array<T, N>::operator[](boost::array<T, N>::size_type) [with T = int; long unsigned int N = 5ul; boost::array<T, N>::reference = int&; boost::array<T, N>::size_type = long unsigned int]': out of range
Backtrace:
 0# boost::assertion_failed_msg(char const*, char const*, char const*, char const*, long)
 1# boost::array<int, 5ul>::operator[](unsigned long)
 2# oops(unsigned long)
 3# bar(int)
 4# foo(int)
 5# bar(int)
 6# foo(int)
 7# bar(int)
 8# foo(int)
 9# bar(int)
10# foo(int)
11# bar(int)
12# foo(int)
13# bar(int)
14# foo(int)
15# main
16# __libc_start_main
17# _start
18# ??
*/

//[getting_started_assert_handlers

// BOOST_ENABLE_ASSERT_DEBUG_HANDLER is defined for the whole project
#include <stdexcept>    // std::logic_error
#include <iostream>     // std::cerr
#include <boost/stacktrace.hpp>

namespace boost {
    void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line) {
        std::cerr << "Expression '" << expr << "' is false in function '" << function << "': " << (msg ? msg : "<...>") << ".\n"
            << "Backtrace:\n" << boost::stacktrace::stacktrace() << '\n';
        std::abort();
    }

    void assertion_failed(char const* expr, char const* function, char const* file, long line) {
        ::boost::assertion_failed_msg(expr, 0 /*nullptr*/, function, file, line);
    }
} // namespace boost
//]


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//[getting_started_terminate_handlers

#include <exception>    // std::set_terminate, std::abort
#include <signal.h>     // ::signal
#include <boost/stacktrace.hpp>
#include <iostream>     // std::cerr

void my_terminate_handler() {
    std::cerr << "Terminate called:\n" << boost::stacktrace::stacktrace() << '\n';
    std::abort();
}

void my_signal_handler(int signum) {
    boost::stacktrace::stacktrace bt;
    if (bt) {
        std::cerr << "Signal " << signum << ", backtrace:\n" << boost::stacktrace::stacktrace() << '\n'; // ``[footnote Strictly speaking this code is not async-signal-safe, but we have SIGSEGV already it could hardly become worse. [link boost_stacktrace.build_macros_and_backends Section "Build, Macros and Backends"] describes async-signal-safe backends, so if you will use the noop backend code becomes absolutely valid as that backens always returns 0 frames and `operator<<` will be never called. ]``
    }
    std::abort();
}
//]

void setup_handlers() {
//[getting_started_setup_handlers
    std::set_terminate(&my_terminate_handler);
    ::signal(SIGSEGV, &my_signal_handler);
//]
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// outputs: 0x401a25,0x401a25,0x401a25,0x401a25,0x401a25,0x401a25,0x4019cb,0x401a7f,0x7f9da8a46e50,0x4013e0,0,

#ifdef BOOST_NO_CXX11_RANGE_BASED_FOR
#include <boost/stacktrace.hpp>
#include <iostream>     // std::cout

namespace bs = boost::stacktrace;
void dump_compact(const bs::stacktrace& st) {
    for (unsigned i = 0; i < st.size(); ++i) {
        bs::frame_view frame = st[i];
        std::cout << frame.address() << ',';
    }

    std::cout << std::endl;
}
#else
//[getting_started_trace_addresses
#include <boost/stacktrace.hpp>
#include <iostream>     // std::cout

namespace bs = boost::stacktrace;
void dump_compact(const bs::stacktrace& st) {
    for (bs::frame_view frame: st) {
        std::cout << frame.address() << ',';
    }

    std::cout << std::endl;
}
//]
#endif

BOOST_NOINLINE boost::stacktrace::stacktrace rec1(int i);
BOOST_NOINLINE boost::stacktrace::stacktrace rec2(int i);

BOOST_NOINLINE boost::stacktrace::stacktrace rec1(int i) {
    if (i < 5) {
        if (!i) return boost::stacktrace::stacktrace();
        return rec2(--i);
    }

    return rec2(i - 2);
}

BOOST_NOINLINE boost::stacktrace::stacktrace rec2(int i) {
    if (i < 5) {
        if (!i) return boost::stacktrace::stacktrace();
        return rec2(--i);
    }

    return rec2(i - 2);
}


#include <boost/core/no_exceptions_support.hpp>
int main() {
    dump_compact(rec1(8));
    setup_handlers();

    BOOST_TRY {
        foo(5); // testing assert handler
    } BOOST_CATCH(...) {
    } BOOST_CATCH_END

}


