// Copyright Antony Polukhin, 2016-2024.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_ADDR_BASE_MSVC_HPP
#define BOOST_STACKTRACE_DETAIL_ADDR_BASE_MSVC_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#ifdef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#else
// Prevent inclusion of extra Windows SDK headers which can cause conflict
// with other code using Windows SDK
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#undef WIN32_LEAN_AND_MEAN
#endif

namespace boost { namespace stacktrace { namespace detail {
  inline  uintptr_t get_own_proc_addr_base(const void* addr) {
        HANDLE processHandle = GetCurrentProcess();
        HMODULE modules[1024];
        
        DWORD needed;

        if (EnumProcessModulesEx(processHandle, modules, sizeof(modules), &needed, LIST_MODULES_ALL)) {
            int moduleCount = needed / sizeof(HMODULE);

            for (int i = 0; i < moduleCount; ++i) {
                MODULEINFO moduleInfo;
                TCHAR moduleName[MAX_PATH];

                // Get the module name
                if (GetModuleBaseName(processHandle, modules[i], moduleName, sizeof(moduleName) / sizeof(TCHAR))) {
                    // Get module information
                    if (GetModuleInformation(processHandle, modules[i], &moduleInfo, sizeof(moduleInfo))) {
                        if (moduleInfo.lpBaseOfDll <= addr && addr < LPBYTE(moduleInfo.lpBaseOfDll) + moduleInfo.SizeOfImage) {
                            // Module contains the address
                            return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
                        }
                    }
                }
            }
        }

        return 0;
    }

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_ADDR_BASE_MSVC_HPP
