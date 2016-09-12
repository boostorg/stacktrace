// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <boost/core/lightweight_test.hpp>

using boost::stacktrace::stacktrace;
BOOST_SYMBOL_IMPORT std::pair<stacktrace, stacktrace> foo2(int i);
BOOST_SYMBOL_IMPORT std::pair<stacktrace, stacktrace> foo1(int i);
BOOST_SYMBOL_IMPORT stacktrace return_from_nested_namespaces();

void test_deeply_nested_namespaces() {
    std::stringstream ss;
    ss << return_from_nested_namespaces();
    std::cout << ss.str() << '\n';
    BOOST_TEST(ss.str().find("main") != std::string::npos);

#if defined(BOOST_STACKTRACE_LINK) || !defined(BOOST_STACKTRACE_USE_BACKTRACE)
    BOOST_TEST(ss.str().find("get_backtrace_from_nested_namespaces") != std::string::npos);
#endif
}

void test_nested() {
    std::pair<stacktrace, stacktrace> res = foo2(15);

    std::stringstream ss1, ss2;
    ss1 << res.first;
    ss2 << res.second;
    std::cout << "'" << ss1.str() << "'\n\n" << ss2.str() << std::endl;
    BOOST_TEST(!ss1.str().empty());
    BOOST_TEST(!ss2.str().empty());

    BOOST_TEST(ss1.str().find(" 0# ") != std::string::npos);
    BOOST_TEST(ss2.str().find(" 0# ") != std::string::npos);

    BOOST_TEST(ss1.str().find(" 1# ") != std::string::npos);
    BOOST_TEST(ss2.str().find(" 1# ") != std::string::npos);

    BOOST_TEST(ss1.str().find("main") != std::string::npos);
    BOOST_TEST(ss2.str().find("main") != std::string::npos);

#if defined(BOOST_STACKTRACE_LINK) || !defined(BOOST_STACKTRACE_USE_BACKTRACE)
    BOOST_TEST(ss1.str().find("foo1") != std::string::npos);
    BOOST_TEST(ss1.str().find("foo2") != std::string::npos);
    BOOST_TEST(ss2.str().find("foo1") != std::string::npos);
    BOOST_TEST(ss2.str().find("foo2") != std::string::npos);
#endif
}


int main() {
    test_deeply_nested_namespaces();
    test_nested();

    return boost::report_errors();
}
