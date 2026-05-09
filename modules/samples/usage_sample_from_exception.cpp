// Copyright Antony Polukhin, 2025-2026.
// Copyright Fedor Osetrov, 2025-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
<<<<<<< HEAD
#include <map>
=======
#include <stdexcept>
>>>>>>> develop

#include <boost/config.hpp>

#include <boost/stacktrace.hpp>

namespace {

BOOST_NOINLINE void foo() {
<<<<<<< HEAD
    std::map<int, int> m;
    std::ignore = m.at(1);
=======
    throw std::logic_error{"Foo"};
>>>>>>> develop
}

BOOST_NOINLINE void bar() {
    foo();
}

}

int main() {
    try {
        bar();
    } catch (const std::exception& ex) {
        const auto trace = boost::stacktrace::stacktrace::from_current_exception();
        std::cerr << "Exception: " << ex.what() << ", trace:\n" << trace << '\n';
    }
}
