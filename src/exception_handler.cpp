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

// Linked implementation
#define BOOST_STACKTRACE_INTERNAL_BUILD_LIBS
#if defined(BOOST_STACKTRACE_USE_NOOP)
    #include <boost/stacktrace/detail/frame_noop.ipp>
#elif defined(BOOST_MSVC) || defined(BOOST_STACKTRACE_USE_WINDBG) || defined(BOOST_STACKTRACE_USE_WINDBG_CACHED)
    #include <boost/stacktrace/detail/frame_msvc.ipp>
#else
    #include <boost/stacktrace/detail/frame_unwind.ipp>
#endif
    #include <boost/stacktrace/safe_dump_to.hpp>

// Requires C++ 11 features
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L)

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

        exception_handler::__C_specific_handler_pfunc exception_handler::__C_specific_handler_Original = nullptr;
        exception_handler::exception_function_handler exception_handler::handler_ = nullptr;

#if defined(WINDOWS_STYLE_EXCEPTION_HANDLING)
        LONG WINAPI exception_handler::__C_specific_handler_Detour(struct _EXCEPTION_RECORD* rec, void* frame, struct _CONTEXT* context, struct _DISPATCHER_CONTEXT* dispatch) {
            unsigned int code = rec->ExceptionCode;
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
            inExceptionCall = false;
            return __C_specific_handler_Original(rec, frame, context, dispatch);
#else
            // Default signal handler for that signal
            ::signal(signum, SIG_DFL);
            // Abort application. (On windows native app cannot auto-retry, as execution continue from same place as before, exception loop continues)
            ::raise(SIGABRT);       // This will trigger also exception handler call, which we are handling, inExceptionCall will guard against double call.
#endif
            inExceptionCall = false;
        }

        exception_handler::exception_handler() BOOST_NOEXCEPT {
        }

        exception_handler::exception_handler(exception_function_handler handler) BOOST_NOEXCEPT {
            init(handler);
        }

        bool exception_handler::init(exception_handler::exception_function_handler handler) BOOST_NOEXCEPT {
            handler_ = handler;
            bool ok = true;

            #if defined(BOOST_WINDOWS)
            HMODULE h = GetModuleHandleA("vcruntime140_clr0400");
            if (!h) {
                return false;
            }
            FARPROC p = GetProcAddress(h, "__C_specific_handler");
            MH_STATUS r = MH_Initialize();

            if ( (r == MH_OK || r == MH_ERROR_ALREADY_INITIALIZED) &&
                MH_CreateHookApi(L"vcruntime140_clr0400", "__C_specific_handler", &__C_specific_handler_Detour, (LPVOID*)&__C_specific_handler_Original) == MH_OK &&
                MH_EnableHook(p) == MH_OK
            )
            {
                return true;
            }

            return false;
            #endif

            #ifndef WINDOWS_STYLE_EXCEPTION_HANDLING
            for (const auto& kv : platform_exception_codes) {
                if (::signal(kv.first, posixSignalHandler) == SIG_ERR) {
                    ok = false;
                }
            }
            #endif

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

#endif //__cplusplus
