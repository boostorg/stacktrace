// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <elfutils/libdwfl.h>

int main() {
    Dwfl_Callbacks callbacks{nullptr, nullptr, nullptr, nullptr};
    Dwfl* dwfl_ = dwfl_begin(&callbacks);
}
