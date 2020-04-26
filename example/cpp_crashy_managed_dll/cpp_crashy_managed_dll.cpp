// Copyright Tarmo Pikaro, 2016-2020.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/stacktrace.hpp>

using namespace System;

public ref class TestCppCrash
{
public:
    static System::String^ getCallStack()
    {
        return gcnew String(to_string(boost::stacktrace::stacktrace()).c_str());
    }
};

