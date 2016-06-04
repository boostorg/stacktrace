// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include <boost/config.hpp>

#include <boost/array.hpp>
#include <boost/aligned_storage.hpp>
#include <iosfwd>

namespace boost { namespace stacktrace {

class stacktrace {
    typename boost::aligned_storage<sizeof(void*) * 250>::type    data_;

public:
    BOOST_STATIC_CONSTEXPR std::size_t max_symbol_width = 256;
    typedef boost::array<char, max_symbol_width>  frame_t;

    stacktrace() BOOST_NOEXCEPT;
    stacktrace(const stacktrace& bt) BOOST_NOEXCEPT;
    stacktrace& operator=(const stacktrace& bt) BOOST_NOEXCEPT;
    ~stacktrace() BOOST_NOEXCEPT;

    std::size_t size() const BOOST_NOEXCEPT;
    frame_t operator[](std::size_t frame) const BOOST_NOEXCEPT;
};

template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const stacktrace& bt) {
    const std::streamsize w = os.width();
    const std::size_t frames = bt.size();
    for (std::size_t i = 0; i < frames; ++i) {
        os.width(2);
        os << i;
        os.width(w);
        os << "# " << bt[i].data()  << '\n';
    }

    return os;
}

}} // namespace boost::stacktrace

