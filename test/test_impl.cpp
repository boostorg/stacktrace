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
void foo1(int i);

void foo2(int i) {
    foo1(i);
}


namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
namespace very_very_very_very_very_very_long_namespace {
    extern stacktrace get_backtrace_from_nested_namespaces() {
        return stacktrace();
    }
}}}}}}}}}}

using namespace very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace
    ::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace
    ::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace
    ::very_very_very_very_very_very_long_namespace;

void test_from_nested_namespaces() {
    std::cout << get_backtrace_from_nested_namespaces() << "\n\n";
}
