//  sym_from_addr.hpp  --------------------------------------------------------------//

//  Copyright 2016 Antony Polukhin

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DETAIL_WINAPI_SYM_FROM_ADDR_HPP
#define BOOST_DETAIL_WINAPI_SYM_FROM_ADDR_HPP

#include <boost/detail/winapi/basic_types.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {
struct _SYMBOL_INFO;

BOOST_SYMBOL_IMPORT boost::detail::winapi::BOOL_ WINAPI SymFromAddr(
    /*HANDLE*/ boost::detail::winapi::HANDLE_ hProcess,
    /*DWORD64*/ boost::detail::winapi::ULONGLONG_ Address,
    /*PDWORD64*/ boost::detail::winapi::ULONGLONG_* Displacement,
    /*PSYMBOL_INFO*/ _SYMBOL_INFO* Symbol);

}
#endif

namespace boost {
namespace detail {
namespace winapi {

typedef struct BOOST_DETAIL_WINAPI_MAY_ALIAS _SYMBOL_INFO {
  ULONG_        SizeOfStruct;
  ULONG_        TypeIndex;
  ULONGLONG_    Reserved[2];
  ULONG_        Index;
  ULONG_        Size;
  ULONGLONG_    ModBase;
  ULONG_        Flags;
  ULONGLONG_    Value;
  ULONGLONG_    Address;
  ULONG_        Register;
  ULONG_        Scope;
  ULONG_        Tag;
  ULONG_        NameLen;
  ULONG_        MaxNameLen;
  CHAR_         Name[1];
} SYMBOL_INFO_, *PSYMBOL_INFO_;

using ::SymFromAddr;

BOOST_FORCEINLINE BOOL_ SymFromAddr(HANDLE_ hProcess, ULONGLONG_ Address, ULONGLONG_* Displacement, PSYMBOL_INFO_ Symbol)
{
    return ::SymFromAddr(hProcess, Address, Displacement, reinterpret_cast< ::_SYMBOL_INFO* >(Symbol));
}

}
}
}

#endif // BOOST_DETAIL_WINAPI_SYM_FROM_ADDR_HPP
