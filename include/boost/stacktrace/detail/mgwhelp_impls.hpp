#ifndef BOOST_STACKTRACE_DETAIL_MGWHELP_IMPLS_HPP
#define BOOST_STACKTRACE_DETAIL_MGWHELP_IMPLS_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/stacktrace/frame.hpp>
#include <boost/detail/winapi/get_current_process.hpp>
#include <windows.h>
#include <dbghelp.h>
#include <Shlobj.h>

namespace boost { namespace stacktrace { namespace detail {

bool initialize_sym(HANDLE process_handle) {
    std::string sym_search_path;

    if( (getenv("_NT_SYMBOL_PATH") == NULL) && (getenv("_NT_ALTERNATE_SYMBOL_PATH") == NULL)) {
        char local_app_data[MAX_PATH];
        HRESULT hr = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, local_app_data);
        if (SUCCEEDED(hr))
            sym_search_path = std::string("srv*") + std::string(local_app_data) + std::string("\\drmingw*http://msdl.microsoft.com/download/symbols");
        else
            sym_search_path = "srv*http://msdl.microsoft.com/download/symbols";
    }

    return SymInitialize(process_handle, sym_search_path.c_str(), true);
}

bool get_line_from_addr( HANDLE process_handle, boost::stacktrace::detail::native_frame_ptr_t addr, std::string& file_name, uint32_t& line_number)
{
    IMAGEHLP_LINE64 line;
    memset(&line, 0, sizeof(IMAGEHLP_LINE64));
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement = 0;

    file_name = "";
    line_number = 0;

    if(!SymGetLineFromAddr64(process_handle, (DWORD64)addr +1, &displacement, &line))
        return false;

    file_name = std::string( line.FileName);
    line_number = line.LineNumber;

    return true;
}

struct to_string_using_mgwhelp {
    bool is_inited;

    std::string file_name;
    uint32_t line_number;
    std::string res;
    HANDLE process_handle;

    to_string_using_mgwhelp() :
        is_inited( false),
        process_handle( boost::detail::winapi::GetCurrentProcess())
    {}

    void prepare_function_name(const void* addr) {
        if( !is_inited)
        {
            if( !initialize_sym( process_handle))
                return;
            is_inited = true;
        }

        boost::stacktrace::detail::get_line_from_addr( process_handle, addr, file_name, line_number);
    }

    bool prepare_source_location(const void* /*addr*/) {
        if (file_name.empty() || !line_number) {
            return false;
        }

        res += " at ";
        res += file_name;
        res += ':';
        res += boost::lexical_cast<boost::array<char, 40> >(line_number).data();
        return true;
    }
};

template <class Base> class to_string_impl_base;
typedef to_string_impl_base<to_string_using_mgwhelp> to_string_impl;

inline std::string name_impl(const void* addr) {
    HANDLE process_handle = boost::detail::winapi::GetCurrentProcess();

    char buffer[ sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)];
    PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)buffer;
    symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol_info->MaxNameLen = MAX_SYM_NAME;

    DWORD64 displacement = 0;
    DWORD options = SymGetOptions();

    if( SymFromAddr(process_handle, (DWORD64)addr, &displacement, symbol_info))
        if (options & SYMOPT_UNDNAME)
            return symbol_info->Name;
        else {
            char result[MAX_SYM_NAME];
            UnDecorateSymbolName( symbol_info->Name, result, MAX_SYM_NAME, UNDNAME_NAME_ONLY);
            return result;
        }
    return std::string();
}

} // namespace detail

std::string frame::source_file() const {
    std::string file_name;
    uint32_t line_number;
    if(boost::stacktrace::detail::get_line_from_addr( boost::detail::winapi::GetCurrentProcess(), addr_, file_name, line_number))
        return file_name;
    else
        return std::string();
}

std::size_t frame::source_line() const {
    std::string file_name;
    uint32_t line_number;
    if(boost::stacktrace::detail::get_line_from_addr( boost::detail::winapi::GetCurrentProcess(), addr_, file_name, line_number))
        return line_number;
    else
        return 0;
}

}} // namespace boost::stacktrace


#endif // BOOST_STACKTRACE_DETAIL_MGWHELP_IMPLS_HPP
