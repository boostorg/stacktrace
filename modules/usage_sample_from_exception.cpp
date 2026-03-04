#include <iostream>
#include <stdexcept>

#include <boost/stacktrace.hpp>

namespace {

__attribute__ ((__noinline__)) __attribute__((__visibility__("default"))) void foo() {
    throw std::logic_error{"Foo"};
}

__attribute__ ((__noinline__)) __attribute__((__visibility__("default"))) void bar() {
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
