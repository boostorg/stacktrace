// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_BACKEND_POSIX_HPP
#define BOOST_STACKTRACE_DETAIL_BACKEND_POSIX_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/core/demangle.hpp>
#include <boost/stacktrace/detail/to_hex_array.hpp>
#include <boost/lexical_cast.hpp>

#if defined(BOOST_STACKTRACE_USE_UNWIND)
#include <unwind.h>
#endif

#include <dlfcn.h>
#include <signal.h>
#include <execinfo.h>
#include <cstdio>

#include <sys/types.h>
#include <sys/wait.h>


namespace boost { namespace stacktrace { namespace detail {

class addr2line_pipe {
    FILE* p;
    pid_t pid;

public:
    explicit addr2line_pipe(const char *flag, const char* exec_path, const char* addr) BOOST_NOEXCEPT
        : p(0)
        , pid(0)
    {
        int pdes[2];
        char prog_name[] = "addr2line";
        char* argp[] = {
            prog_name,
            const_cast<char*>(flag),
            const_cast<char*>(exec_path),
            const_cast<char*>(addr),
            0
        };

        if (pipe(pdes) < 0) {
            return;
        }

        pid = fork();
        switch (pid) {
        case -1:
            // failed
            close(pdes[0]);
            close(pdes[1]);
            return;

        case 0:
            // we are the child
            close(STDERR_FILENO);
            close(pdes[0]);
            if (pdes[1] != STDOUT_FILENO) {
                dup2(pdes[1], STDOUT_FILENO);
            }
            execvp(prog_name, argp);
            _exit(127);
        }

        p = fdopen(pdes[0], "r");
        close(pdes[1]);
    }

    operator FILE*() const BOOST_NOEXCEPT {
        return p;
    }

    ~addr2line_pipe() BOOST_NOEXCEPT {
        if (p) {
            fclose(p);
            int pstat = 0;
            kill(pid, SIGKILL);
            waitpid(pid, &pstat, 0);
        }
    }
};

inline std::string addr2line(const char* flag, const void* addr) {
    std::string res;

    Dl_info dli;
    if (!!dladdr(addr, &dli) && dli.dli_fname) {
        res = dli.dli_fname;
    } else {
        res.resize(16);
        int rlin_size = readlink("/proc/self/exe", &res[0], res.size() - 1);
        while (rlin_size == static_cast<int>(res.size() - 1)) {
            res.resize(res.size() * 4);
            rlin_size = readlink("/proc/self/exe", &res[0], res.size() - 1);
        }
        if (rlin_size == -1) {
            res.clear();
            return res;
        }
        res.resize(rlin_size);
    }

    addr2line_pipe p(flag, res.c_str(), to_hex_array(addr).data());
    res.clear();

    if (!p) {
        return res;
    }

    char data[32];
    while (!std::feof(p)) {
        if (std::fgets(data, sizeof(data), p)) {
            res += data;
        } else {
            break;
        }
    }

    // Trimming
    while (!res.empty() && (res[res.size() - 1] == '\n' || res[res.size() - 1] == '\r')) {
        res.erase(res.size() - 1);
    }

    return res;
}

inline std::string try_demangle(const char* mangled) {
    std::string res;

    boost::core::scoped_demangled_name demangled(mangled);
    if (demangled.get()) {
        res = demangled.get();
    } else {
        res = mangled;
    }

    return res;
}




#if defined(BOOST_STACKTRACE_USE_UNWIND)
struct unwind_state {
    void** current;
    void** end;
};

inline _Unwind_Reason_Code unwind_callback(struct _Unwind_Context* context, void* arg) {
    unwind_state* state = static_cast<unwind_state*>(arg);
    *state->current = reinterpret_cast<void*>(
        _Unwind_GetIP(context)
    );

    ++state->current;
    if (!*(state->current - 1) || state->current == state->end) {
        return _URC_END_OF_STACK;
    }
    return _URC_NO_REASON;
}
#endif




std::size_t backend::collect(void** memory, std::size_t size) BOOST_NOEXCEPT {
    std::size_t frames_count = 0;
    if (!size) {
        return frames_count;
    }
#if defined(BOOST_STACKTRACE_USE_UNWIND)
    unwind_state state = { memory, memory + size };
    _Unwind_Backtrace(&unwind_callback, &state);
    frames_count = state.current - memory;
#elif defined(BOOST_STACKTRACE_USE_BACKTRACE)
    frames_count = ::backtrace(memory, size);
#else
#   error No stacktrace backend defined. Define BOOST_STACKTRACE_USE_UNWIND or BOOST_STACKTRACE_USE_BACKTRACE
#endif
    if (memory[frames_count - 1] == 0) {
        -- frames_count;
    }

    return frames_count;
}

std::string backend::to_string(const void* addr) {
    std::string res = boost::stacktrace::frame(addr).name();
    if (res.empty()) {
        res = to_hex_array(addr).data();
    }
    
    std::string source_line = boost::stacktrace::detail::addr2line("-Cpe", addr);
    if (!source_line.empty() && source_line[0] != '?') {
        res += " at ";
        res += source_line;
    } else {
        Dl_info dli;
        if (!!dladdr(addr, &dli) && dli.dli_sname) {
            res += " in ";
            res += dli.dli_fname;
        }
    }
    
    return res;
    //return addr2line("-Cfipe", addr); // Does not seem to work in all cases
}

std::string backend::to_string(const frame* frames, std::size_t size) {
    std::string res;
    res.reserve(64 * size);
    for (std::size_t i = 0; i < size; ++i) {
        if (i < 10) {
            res += ' ';
        }
        res += boost::lexical_cast<boost::array<char, 40> >(i).data();
        res += '#';
        res += ' ';
        res += to_string(frames[i].address());
        res += '\n';
    }

    return res;
}

} // namespace detail

std::string frame::name() const {
    std::string res;

    Dl_info dli;
    if (!!dladdr(addr_, &dli) && dli.dli_sname) {
        res = boost::stacktrace::detail::try_demangle(dli.dli_sname);
    } else {
        res = boost::stacktrace::detail::addr2line("-fe", addr_);
        res = res.substr(0, res.find_last_of('\n'));
        res = boost::stacktrace::detail::try_demangle(res.c_str());
    }

    if (res == "??") {
        res.clear();
    }

    return res;
}

std::string frame::source_file() const {
    std::string res = boost::stacktrace::detail::addr2line("-e", addr_);
    res = res.substr(0, res.find_last_of(':'));
    if (res == "??") {
        res.clear();
    }
    return res;
}

std::size_t frame::source_line() const {
    std::string res = boost::stacktrace::detail::addr2line("-e", addr_);
    const std::size_t last = res.find_last_of(':');
    if (last == std::string::npos) {
        return 0;
    }
    res = res.substr(last + 1);

    std::size_t line_num = 0;
    if (!boost::conversion::try_lexical_convert(res, line_num)) {
        return 0;
    }

    return line_num;
}


}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_BACKEND_POSIX_HPP
