// Copyright Romain Geissler, 2025.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>

int main() {
#ifdef __GLIBCXX__
    return 0;
#else
#error "The C++ runtime library isn't libstdc++"
#endif
}
