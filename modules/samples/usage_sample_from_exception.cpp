#include <iostream>
#include <stdexcept>

#include <boost/config.hpp>

#include <boost/stacktrace.hpp>

namespace {

BOOST_NOINLINE void foo() {
    throw std::logic_error{"Foo"};
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
