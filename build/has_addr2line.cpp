// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <string>

int main(int /*argc*/, const char* argv[]) {
    std::string s = "addr2line -e " + argv[0] + " 0";
    return std::system(s.c_str());
}
