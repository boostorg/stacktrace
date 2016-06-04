// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>

using namespace boost::stacktrace;
typedef std::pair<stacktrace, stacktrace> btp;
void foo2(int i);

void foo1(int i) {
    if (i) {
        foo2(i - 1);
        return;
    }

    btp ret;
    try {
        ret.first = stacktrace();
        throw std::logic_error("test");
    } catch (const std::logic_error& e) {
        ret.second = stacktrace();
        std::cout << e.what() << '\n';
        throw ret;
    }
}



void test_nested() {
    btp res;
    try {
        foo2(15);
    } catch(btp const r) {
        res = r;
    }

    std::stringstream ss1, ss2;
    ss1 << res.first;
    ss2 << res.second;
    std::cout << ss1.str() << "\n\n" << ss2.str() << std::endl;
    assert(!ss1.str().empty());
    assert(!ss2.str().empty());

    assert(ss1.str().find(" 0# ") != std::string::npos);
    assert(ss2.str().find(" 0# ") != std::string::npos);

    assert(ss1.str().find(" 1# ") != std::string::npos);
    assert(ss2.str().find(" 1# ") != std::string::npos);

    assert(ss1.str().find("main") != std::string::npos);
    assert(ss2.str().find("main") != std::string::npos);
}

void test_from_nested_namespaces();

int main() {
    test_from_nested_namespaces();
    test_nested();
}
