// Copyright Tarmo Pikaro, 2020.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_EXCEPTION_HANDLER_HPP
#define BOOST_EXCEPTION_HANDLER_HPP
#ifdef BOOST_STACKTRACE

#include <boost/config.hpp>
#include <functional>
#include <map>

#if defined(BOOST_WINDOWS)
    #define WINDOWS_STYLE_EXCEPTION_HANDLING
#endif

#ifdef WINDOWS_STYLE_EXCEPTION_HANDLING
    #include <windows.h>                    //WINAPI, etc...
#endif

namespace boost {
    namespace stacktrace {

        /// Low level exception information
        class low_level_exception_info
        {
        public:
            unsigned int code;
            const char* name;
            bool handled;
        };

        /// Registers exception handler
        class exception_handler
        {
        public:
            typedef std::function<void(low_level_exception_info& ex_info)> exception_function_handler;
        public:
            static const std::map<unsigned int, const char*> platform_exception_codes;

            exception_handler() BOOST_NOEXCEPT;
            exception_handler(exception_function_handler handler) BOOST_NOEXCEPT;
            bool init(exception_function_handler handler) BOOST_NOEXCEPT;
            void deinit() BOOST_NOEXCEPT;
            ~exception_handler() BOOST_NOEXCEPT;

        private:
            static exception_function_handler handler_;

#if defined(WINDOWS_STYLE_EXCEPTION_HANDLING)
            using __C_specific_handler_pfunc = LONG(WINAPI*)(struct _EXCEPTION_RECORD*, void*, struct _CONTEXT*, struct _DISPATCHER_CONTEXT*);
            static __C_specific_handler_pfunc __C_specific_handler_Original;

            static LONG WINAPI __C_specific_handler_Detour(struct _EXCEPTION_RECORD* rec, void* frame, struct _CONTEXT* context, struct _DISPATCHER_CONTEXT* dispatch);
#else
            static void posixSignalHandler(int signum) BOOST_NOEXCEPT;
#endif
        };
    };
};

#endif //BOOST_STACKTRACE
#endif //BOOST_EXCEPTION_HANDLER_HPP


