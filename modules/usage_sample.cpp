#include <iostream>

import boost.stacktrace;

namespace {

void foo() {
    std::cerr << boost::stacktrace::stacktrace{};
}

void bar() {
    foo();
}

}

int main() {
    bar();

    return 0;
}
