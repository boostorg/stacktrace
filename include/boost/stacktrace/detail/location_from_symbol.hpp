// Copyright Antony Polukhin, 2016-2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_LOCATION_FROM_SYMBOL_HPP
#define BOOST_STACKTRACE_DETAIL_LOCATION_FROM_SYMBOL_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
#   include <dlfcn.h>
#else
#   include <boost/winapi/dll.hpp>
#endif

#ifdef _AIX
/* AIX doesn't provide dladdr syscall.
   This provides a minimal implementation of dladdr which retrieves
   only files information.
   TODO: Implement the symbol name.  */

#include <sys/ldr.h>
#include <sys/debug.h>
#include <cstring>

#define DL_BUFF_SIZE 0x1000

extern "C" {
  typedef struct
  {
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
  } Dl_info;

  int dladdr(void* address, Dl_info* info) {
    void *buff = malloc (DL_BUFF_SIZE);

    info->dli_fname = NULL;
    info->dli_fbase = NULL;
    info->dli_sname = NULL;
    info->dli_saddr = NULL;

    if (loadquery (L_GETINFO, buff, DL_BUFF_SIZE) != -1 ) {
      struct ld_info *pld_info = (struct ld_info*) buff;

      while (1) {
	if (((char*) address >= (char*) pld_info->ldinfo_dataorg &&
	     (char*) address < (char*) pld_info->ldinfo_dataorg + pld_info-> ldinfo_datasize )
	    || (( char*) address >= (char*) pld_info->ldinfo_textorg &&
		(char*) address < (char*) pld_info->ldinfo_textorg +  pld_info-> ldinfo_textsize )){


	  /* ldinfo_filename is the null-terminated path name followed
	     by null-terminated member name.
	     If the file is not an archive, then member name is null. */
	  uint size_filename = strlen (pld_info->ldinfo_filename);
	  uint size_member = strlen (pld_info->ldinfo_filename + size_filename + 1);

	  /* If member is not null, '(' and ')' must be added to create a
	     fname looking like "filename(membername)".  */
	  char *fname = (char*) malloc (size_filename + (size_member ? size_member  + 3 : 1));
	  strcpy (fname, pld_info->ldinfo_filename);
	  if (size_member) {
	    strcat (fname, "(");
	    strcat (fname, pld_info->ldinfo_filename + size_filename + 1);
	    strcat (fname, ")");
	  }

	  info->dli_fname = fname;
	  info->dli_fbase = pld_info->ldinfo_textorg;

	  free (buff);
	  return 1;
	}

	if (!pld_info->ldinfo_next) {
	  free (buff);
	  return 0;
	}
	pld_info = (struct ld_info *) ((char*) pld_info + pld_info->ldinfo_next);
      }
    }
    return 0;
  }
}

#endif

namespace boost { namespace stacktrace { namespace detail {

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
class location_from_symbol {
    ::Dl_info dli_;

public:
    explicit location_from_symbol(const void* addr) BOOST_NOEXCEPT
        : dli_()
    {
        if (!::dladdr(const_cast<void*>(addr), &dli_)) { // `dladdr` on Solaris accepts nonconst addresses
            dli_.dli_fname = 0;
        }
    }

    bool empty() const BOOST_NOEXCEPT {
        return !dli_.dli_fname;
    }

    const char* name() const BOOST_NOEXCEPT {
        return dli_.dli_fname;
    }
};

class program_location {
public:
    const char* name() const BOOST_NOEXCEPT {
        return 0;
    }
};

#else

class location_from_symbol {
    BOOST_STATIC_CONSTEXPR boost::winapi::DWORD_ DEFAULT_PATH_SIZE_ = 260;
    char file_name_[DEFAULT_PATH_SIZE_];

public:
    explicit location_from_symbol(const void* addr) BOOST_NOEXCEPT {
        file_name_[0] = '\0';

        boost::winapi::MEMORY_BASIC_INFORMATION_ mbi;
        if (!boost::winapi::VirtualQuery(addr, &mbi, sizeof(mbi))) {
            return;
        }

        boost::winapi::HMODULE_ handle = reinterpret_cast<boost::winapi::HMODULE_>(mbi.AllocationBase);
        if (!boost::winapi::GetModuleFileNameA(handle, file_name_, DEFAULT_PATH_SIZE_)) {
            file_name_[0] = '\0';
            return;
        }
    }

    bool empty() const BOOST_NOEXCEPT {
        return file_name_[0] == '\0';
    }

    const char* name() const BOOST_NOEXCEPT {
        return file_name_;
    }
};

class program_location {
    BOOST_STATIC_CONSTEXPR boost::winapi::DWORD_ DEFAULT_PATH_SIZE_ = 260;
    char file_name_[DEFAULT_PATH_SIZE_];

public:
    program_location() BOOST_NOEXCEPT {
        file_name_[0] = '\0';

        const boost::winapi::HMODULE_ handle = 0;
        if (!boost::winapi::GetModuleFileNameA(handle, file_name_, DEFAULT_PATH_SIZE_)) {
            file_name_[0] = '\0';
        }
    }

    const char* name() const BOOST_NOEXCEPT {
        return file_name_[0] ? file_name_ : 0;
    }
};
#endif

}}} // namespace boost::stacktrace::detail

#endif // BOOST_STACKTRACE_DETAIL_LOCATION_FROM_SYMBOL_HPP
