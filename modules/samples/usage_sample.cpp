#include <iostream>
#include <boost/config.hpp>

#if defined(STACKTRACE_MODULE)
import STACKTRACE_MODULE;
#else
import boost.stacktrace;
#endif

namespace {

BOOST_NOINLINE void foo() {
    auto trace = boost::stacktrace::stacktrace{};
    std::cerr << trace;
}

BOOST_NOINLINE void bar() {
    foo();
}

}

int main() {
    bar();

    return 0;
}
