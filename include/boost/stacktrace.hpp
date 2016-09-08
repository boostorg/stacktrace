// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <boost/aligned_storage.hpp>
#include <iosfwd>
#include <string>

/// @cond
#if defined(BOOST_STACKTRACE_HEADER_ONLY)
#   define BOOST_STACKTRACE_FUNCTION inline
#else
#   define BOOST_STACKTRACE_FUNCTION
#endif
/// @endcond

namespace boost { namespace stacktrace {

class stacktrace {
    BOOST_STATIC_CONSTEXPR std::size_t max_implementation_size = sizeof(void*) * 110u;
    boost::aligned_storage<max_implementation_size>::type impl_;
public:
    /// Stores the current function call sequence inside the class
    BOOST_STACKTRACE_FUNCTION stacktrace() BOOST_NOEXCEPT;

    BOOST_STACKTRACE_FUNCTION stacktrace(const stacktrace& bt) BOOST_NOEXCEPT;
    BOOST_STACKTRACE_FUNCTION stacktrace& operator=(const stacktrace& bt) BOOST_NOEXCEPT;
    BOOST_STACKTRACE_FUNCTION ~stacktrace() BOOST_NOEXCEPT;

    /// @returns Number of function names stored inside the class.
    BOOST_STACKTRACE_FUNCTION std::size_t size() const BOOST_NOEXCEPT;

    /// @param frame Zero based index of function to return. 0
    /// is the function index where stacktrace was constructed and
    /// index close to this->size() contains function `main()`.
    /// @returns Function name in a human readable form.
    /// @throws std::bad_alloc if not enough memory.
    BOOST_STACKTRACE_FUNCTION std::string operator[](std::size_t frame) const;
};


/// Outputs stacktrace in a human readable format to output stream.
template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const stacktrace& bt) {
    const std::streamsize w = os.width();
    const std::size_t frames = bt.size();
    for (std::size_t i = 0; i < frames; ++i) {
        os.width(2);
        os << i;
        os.width(w);
        const std::string f = bt[i];
        os << "# ";
        if (f.empty()) {
            os << "??";
        } else {
            os << f;
        }
        os << '\n';
    }

    return os;
}

}} // namespace boost::stacktrace

/// @cond
#undef BOOST_STACKTRACE_FUNCTION

#if defined(BOOST_STACKTRACE_HEADER_ONLY)
#   include <boost/stacktrace/detail/stacktrace.ipp>
#endif
/// @endcond
