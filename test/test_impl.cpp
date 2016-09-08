// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <stdexcept>

using namespace boost::stacktrace;
BOOST_SYMBOL_EXPORT BOOST_NOINLINE std::pair<stacktrace, stacktrace> foo1(int i);
BOOST_SYMBOL_EXPORT BOOST_NOINLINE std::pair<stacktrace, stacktrace> foo2(int i);

std::pair<stacktrace, stacktrace> foo1(int i) {
    if (i) {
        return foo2(i - 1);
    }

    std::pair<stacktrace, stacktrace> ret;
    try {
        throw std::logic_error("test");
    } catch (const std::logic_error& e) {
        ret.second = stacktrace();
        return ret;
    }
}

std::pair<stacktrace, stacktrace> foo2(int i) {
    if (i) {
        return foo1(--i);
    } else {
        return foo1(i);
    }
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
    BOOST_SYMBOL_EXPORT BOOST_NOINLINE stacktrace get_backtrace_from_nested_namespaces() {
        return stacktrace();
    }
}}}}}}}}}}

BOOST_SYMBOL_EXPORT BOOST_NOINLINE stacktrace return_from_nested_namespaces() {
    using very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace
        ::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace
        ::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace::very_very_very_very_very_very_long_namespace
        ::very_very_very_very_very_very_long_namespace::get_backtrace_from_nested_namespaces;

    return get_backtrace_from_nested_namespaces();
}
