// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/array.hpp>
BOOST_NOINLINE void foo(int i);
BOOST_NOINLINE void bar(int i);
 
BOOST_NOINLINE void bar(int i) {
    boost::array<int, 5> a = {{-1, -231, -123, -23, -32}};
    if (i >= 0) {
        foo(a[i]);
    } else {
        std::terminate();
    }
}

BOOST_NOINLINE void foo(int i) {
    bar(--i);
}

// Ignoring _Exit for testing purposes
#define _Exit(x) exit(x+1)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//[getting_started_terminate_handlers

#include <signal.h>     // ::signal
#include <cstdlib>      // std::_Exit
#include <boost/stacktrace.hpp>

void my_signal_handler(int signum) {
    ::signal(signum, SIG_DFL);
    boost::stacktrace::this_thread_frames::dump("./backtrace.dump");
    std::_Exit(-1);
}
//]

void setup_handlers() {
//[getting_started_setup_handlers
    ::signal(SIGSEGV, &my_signal_handler);
    ::signal(SIGABRT, &my_signal_handler);
//]
}


#include <iostream>     // std::cerr
#include <boost/filesystem.hpp>

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        // We are copying files to make sure that stacktrace printing works independently from executable name
        {
            boost::filesystem::path command_1 = argv[0];
            command_1 = command_1.parent_path() / (command_1.stem().string() + "_1") / command_1.extension();
            boost::filesystem::copy_file(argv[0], command_1, boost::filesystem::copy_option::overwrite_if_exists);
            command_1 += " 1";
            if (std::system(command_1.c_str())) {
                std::exit(-1);
            }
        }

        {
            boost::filesystem::path command_2 = argv[0];
            command_2 = command_2.parent_path() / (command_2.stem().string() + "_2") / command_2.extension();
            boost::filesystem::copy_file(argv[0], command_2, boost::filesystem::copy_option::overwrite_if_exists);
            command_2 += " 2";
            if (std::system(command_2.c_str())) {
                std::exit(-2);
            }
        }

        return 0;
    }

    switch(argv[1][0]) {
    case '1':
        setup_handlers();
        foo(5);
        return -11;
    case '2':
        boost::stacktrace::stacktrace st = boost::stacktrace::stacktrace::from_dump("./backtrace.dump");
        std::cout << st << std::endl;
        return 0;
    }
    return 3;
}


