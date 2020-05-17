// Copyright Tarmo Pikaro, 2020.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#pragma unmanaged
#include <boost/stacktrace.hpp>
#include <boost/stacktrace/exception_handler.hpp>

std::string getCallStack()
{
    return to_string(boost::stacktrace::stacktrace());
}

// Must reside behind "#pragma unmanaged", otherwise crash will be handled by managed exception handler.
void crashingFunction()
{
    int* p = nullptr;
    *p = 0;
}

// Performs jump by null pointer. Call stack is non tracable (dont know where we were at the moment of exception)
void nativeJumpByNullFunction()
{
    void(*func)(void) = nullptr;
    func();
}

#ifdef _MSC_VER
    #pragma warning(push)
    // warning C4717: 'nativeStackOverflowFunction': recursive on all control paths, function will cause runtime stack overflow
    #pragma warning(disable: 4717)
#endif

void nativeStackOverflowFunction()
{
    nativeStackOverflowFunction();
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

void throwManagedException(const std::string& msg);

// C++ catches exception, but it can also see where crash occurred
void crashingFunctionBehindNativeCatch()
{
    try {
        crashingFunction();
    }
    catch (...) {
        throwManagedException("crashingFunctionBehindNativeCatch: crashed");
    }
}


#pragma managed


public ref class NativeException : public System::Exception
{
public:
    System::String^ _stackTrace;
    System::String^ _message;

    NativeException(const std::string message, const std::string stackTrace)
    {
        _stackTrace = gcnew System::String(stackTrace.c_str());
        _message = gcnew System::String(message.c_str());
    }

    property System::String^ StackTrace
    {
        System::String^ get() override
        {
            return _stackTrace;
        }
    }

    property System::String^ Message
    {
        System::String^ get() override
        {
            return _message;
        }
    }

    System::String^ ToString() override
    {
        return _message + "\n" + _stackTrace;
    }
};

#ifdef BOOST_STACKTRACE
void throwDotNetException(boost::stacktrace::low_level_exception_info& ex_info)
{
    throw gcnew NativeException(ex_info.name, getCallStack());
}
#endif

void throwManagedException(const std::string& msg)
{
    throw gcnew NativeException(msg, getCallStack());
}


void enableCppUnhandledExceptionDispatcher(bool b) {
#ifndef BOOST_STACKTRACE
    b;
#else
    static boost::stacktrace::exception_handler handler(throwDotNetException);

    if (!b)
    {
        handler.deinit();
    }
#endif
}


using namespace System;



public ref class TestCppCrash
{
public:
    static System::String^ getCallStack()
    {
        return gcnew String(::getCallStack().c_str());
    }

    static void enableCppUnhandledExceptionDispatcher(bool b) {
        ::enableCppUnhandledExceptionDispatcher(b);
    }

    static void crashingFunction() {
        ::crashingFunction();
    }

    static void nativeJumpByNullFunction() {
        ::nativeJumpByNullFunction();
    }

    static void nativeStackOverflowFunction() {
        ::nativeStackOverflowFunction();
    }

    static void crashingFunctionBehindNativeCatch() {
        ::crashingFunctionBehindNativeCatch();
    }

};

