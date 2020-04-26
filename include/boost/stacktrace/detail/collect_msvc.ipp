// Copyright Antony Polukhin, 2016-2019.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_COLLECT_MSVC_IPP
#define BOOST_STACKTRACE_DETAIL_COLLECT_MSVC_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/safe_dump_to.hpp>

#include <boost/winapi/stack_backtrace.hpp>
#include <windows.h>                            //CONTEXT, RtlCaptureContext

namespace boost { namespace stacktrace { namespace detail {

std::size_t this_thread_frames::collect(native_frame_ptr_t* out_frames, std::size_t max_frames_count, std::size_t skip) BOOST_NOEXCEPT {
    std::size_t iframe = 0;
    std::size_t max_frame = max_frames_count + skip;

#ifdef _WIN64
    CONTEXT ContextRecord;
    RtlCaptureContext(&ContextRecord);

    for (; iframe < max_frame; iframe++)
    {
        DWORD64 ImageBase;
        PRUNTIME_FUNCTION pFunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip, &ImageBase, NULL);

        if (pFunctionEntry == NULL)
        {
            break;
        }

        PVOID HandlerData;
        DWORD64 EstablisherFrame;
        RtlVirtualUnwind(0 /*UNW_FLAG_NHANDLER*/,
            ImageBase,
            ContextRecord.Rip,
            pFunctionEntry,
            &ContextRecord,
            &HandlerData,
            &EstablisherFrame,
            NULL);

        if (iframe < skip)
        {
            continue;
        }

        out_frames[iframe - skip] = (PVOID)ContextRecord.Rip;
    }
#else
    //
    //  This approach was taken from StackInfoManager.cpp / FillStackInfo
    //  http://www.codeproject.com/Articles/11221/Easy-Detection-of-Memory-Leaks
    //  - slightly simplified the function itself.
    //
    int regEBP;
    __asm mov regEBP, ebp;

    long* pFrame = (long*)regEBP;           // pointer to current function frame
    void* pNextInstruction;

    //
    // Using __try/_catch is faster than using ReadProcessMemory or VirtualProtect.
    // We return whatever frames we have collected so far after exception was encountered.
    //
    __try {
        for (; iframe < max_frame; iframe++)
        {
            pNextInstruction = (void*)(*(pFrame + 1));

            if (!pNextInstruction)     // Last frame
            {
                break;
            }

            if (iframe < skip)
            {
                continue;
            }

            out_frames[iframe - skip] = pNextInstruction;
            pFrame = (long*)(*pFrame);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
#endif
    if (iframe <= skip)
    {
        return 0;
    }

    return iframe - skip;
}


}}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_COLLECT_MSVC_IPP
