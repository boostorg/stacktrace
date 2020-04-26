// Copyright Tarmo Pikaro, 2020.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#pragma unmanaged
#include <boost/stacktrace.hpp>

std::string getCallStack()
{
    return to_string(boost::stacktrace::stacktrace());
}

#pragma managed

using namespace System;

public ref class TestCppCrash
{
public:
    static System::String^ getCallStack()
    {
        return gcnew String(::getCallStack().c_str());
    }
};

