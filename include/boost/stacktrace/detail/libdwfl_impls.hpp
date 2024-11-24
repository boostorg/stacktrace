// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_LIBDWFL_IMPLS_HPP
#define BOOST_STACKTRACE_DETAIL_LIBDWFL_IMPLS_HPP

#include <elfutils/libdwfl.h>

#include <boost/stacktrace/detail/to_dec_array.hpp>
#include <boost/stacktrace/frame.hpp>

namespace boost { namespace stacktrace { namespace detail {

class dwfl_handle {
public:
    dwfl_handle() noexcept
        : dwfl_(dwfl_begin(&callbacks_))
    {
        if (dwfl_) {
            dwfl_linux_proc_report(dwfl_, getpid());
            dwfl_report_end(dwfl_, nullptr, nullptr);
        }
    }

    ~dwfl_handle() {
        if (dwfl_) {
            dwfl_end(dwfl_);
        }
    }

    const char* function(Dwarf_Addr addr) const noexcept {
        if (!dwfl_ || !addr) {
            return nullptr;
        }

        Dwfl_Module* dwfl_module = dwfl_addrmodule (dwfl_, addr);
        return dwfl_module ? dwfl_module_addrname(dwfl_module, addr) : nullptr;
    }

    std::pair<const char*, std::size_t> source(Dwarf_Addr addr) const noexcept {
        if (!dwfl_ || !addr) {
            return {nullptr, 0};
        }

        Dwfl_Line* dwfl_line = dwfl_getsrc(dwfl_, addr);
        if (!dwfl_line) {
            return {nullptr, 0};
        }

        int line{0};
        const char* filename = dwfl_lineinfo(dwfl_line, nullptr, &line, nullptr, nullptr, nullptr);
        return {filename, static_cast<std::size_t>(line)};
    }

private:
    Dwfl_Callbacks callbacks_{
        .find_elf = dwfl_linux_proc_find_elf,
        .find_debuginfo = dwfl_build_id_find_debuginfo,
        .section_address = dwfl_offline_section_address,
        .debuginfo_path = nullptr,
    };
    Dwfl* dwfl_;
};

struct to_string_using_dwfl {
    std::string res;
    dwfl_handle dwfl;

    void prepare_function_name(const void* addr) noexcept {
        const char* function = dwfl.function(reinterpret_cast<Dwarf_Addr>(addr));
        if (function) {
            res = function;
        }
    }

    bool prepare_source_location(const void* addr) noexcept {
        auto [filename, line] = dwfl.source(reinterpret_cast<Dwarf_Addr>(addr));
        if (!filename) {
            return false;
        }

        res += " at ";
        res += filename;
        res += ':';
        res += boost::stacktrace::detail::to_dec_array(line).data();

        return true;
    }
};

template <class Base> class to_string_impl_base;
typedef to_string_impl_base<to_string_using_dwfl> to_string_impl;

inline std::string name_impl(const void* addr) {
    dwfl_handle dwfl;
    const char* function = dwfl.function(reinterpret_cast<Dwarf_Addr>(addr));
    return function ? std::string{function} : std::string{};
}

} // namespace detail

std::string frame::source_file() const {
    detail::dwfl_handle dwfl;
    auto [filename, _] = dwfl.source(reinterpret_cast<Dwarf_Addr>(addr_));
    return filename ? std::string{filename} : std::string{};
}

std::size_t frame::source_line() const {
    detail::dwfl_handle dwfl;
    return dwfl.source(reinterpret_cast<Dwarf_Addr>(addr_)).second;
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_LIBDWFL_IMPLS_HPP
