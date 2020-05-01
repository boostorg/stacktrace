// Copyright Tarmo Pikaro, 2020.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stacktrace/exception_handler.hpp>

#ifdef WINDOWS_STYLE_EXCEPTION_HANDLING
    #include <minhook/include/MinHook.h>
#else
    // Partially works on windows as well
    #include <signal.h>     // ::signal, ::raise
#endif


namespace boost {

    namespace stacktrace {

        #define STACKTRACE_EXCEPTION_STRING(Code) { Code, #Code },

        const std::map<unsigned int, const char*> exception_handler::platform_exception_codes =
        {
        #if defined(WINDOWS_STYLE_EXCEPTION_HANDLING)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_ACCESS_VIOLATION)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_BREAKPOINT)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_SINGLE_STEP)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_DENORMAL_OPERAND)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_DIVIDE_BY_ZERO)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_INEXACT_RESULT)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_INVALID_OPERATION)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_OVERFLOW)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_STACK_CHECK)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_FLT_UNDERFLOW)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_INT_DIVIDE_BY_ZERO)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_INT_OVERFLOW)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_PRIV_INSTRUCTION)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_IN_PAGE_ERROR)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_ILLEGAL_INSTRUCTION)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_NONCONTINUABLE_EXCEPTION)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_STACK_OVERFLOW)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_INVALID_DISPOSITION)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_GUARD_PAGE)
                STACKTRACE_EXCEPTION_STRING(EXCEPTION_INVALID_HANDLE)
        #else
                STACKTRACE_EXCEPTION_STRING(SIGABRT)
                STACKTRACE_EXCEPTION_STRING(SIGSEGV)
                STACKTRACE_EXCEPTION_STRING(SIGILL)
                STACKTRACE_EXCEPTION_STRING(SIGFPE)
        #endif
        };

        exception_handler::exception_function_handler exception_handler::handler_ = nullptr;

#if defined(WINDOWS_STYLE_EXCEPTION_HANDLING)
#else
        void exception_handler::posixSignalHandler(int signum) BOOST_NOEXCEPT {
            unsigned int code = signum;

#endif
            const char* str = "Unknown exception code";
            auto it = exception_handler::platform_exception_codes.find(code);
            if (it != exception_handler::platform_exception_codes.end()) {
                str = it->second;
            }

            static bool inExceptionCall = false;

            low_level_exception_info exinfo;
            exinfo.code = code;
            exinfo.name = str;
            exinfo.handled = false;
            if (handler_ != nullptr && !inExceptionCall)
            {
                inExceptionCall = true;
                handler_(exinfo);
            }

#if defined(WINDOWS_STYLE_EXCEPTION_HANDLING)
#else
            // Default signal handler for that signal
            ::signal(signum, SIG_DFL);
            // Abort application. (On windows native app cannot auto-retry, as execution continue from same place as before, exception loop continues)
            ::raise(SIGABRT);       // This will trigger also exception handler call, which we are handling, inExceptionCall will guard against double call.
#endif
            inExceptionCall = false;
        }

        exception_handler::exception_handler() BOOST_NOEXCEPT
        {
        }

        exception_handler::exception_handler(exception_function_handler handler) BOOST_NOEXCEPT
        {
            init(handler);
        }

        bool exception_handler::init(exception_handler::exception_function_handler handler) BOOST_NOEXCEPT {
            handler_ = handler;
            bool ok = true;

            for (const auto& kv : platform_exception_codes) {
                if (::signal(kv.first, posixSignalHandler) == SIG_ERR) {
                    ok = false;
                }
            }

            return ok;
        }

        void exception_handler::deinit() BOOST_NOEXCEPT {
            handler_ = nullptr;
        }
        
        exception_handler::~exception_handler() BOOST_NOEXCEPT {
            deinit();
        }

    };
};
