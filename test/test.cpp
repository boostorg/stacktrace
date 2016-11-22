// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/stacktrace.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <boost/core/lightweight_test.hpp>

using boost::stacktrace::stacktrace;
using boost::stacktrace::frame_view;

#ifdef BOOST_STACKTRACE_DYN_LINK
#   define BOOST_ST_API BOOST_SYMBOL_IMPORT
#else
#   define BOOST_ST_API
#endif

BOOST_ST_API std::pair<stacktrace, stacktrace> foo2(int i);
BOOST_ST_API std::pair<stacktrace, stacktrace> foo1(int i);
BOOST_ST_API stacktrace return_from_nested_namespaces();

void test_deeply_nested_namespaces() {
    std::stringstream ss;
    ss << return_from_nested_namespaces();
    std::cout << ss.str() << '\n';
    BOOST_TEST(ss.str().find("main") != std::string::npos);
std::cerr << "cerr 1\n";
#if defined(BOOST_STACKTRACE_DYN_LINK) || !defined(BOOST_STACKTRACE_USE_BACKTRACE)
    BOOST_TEST(ss.str().find("get_backtrace_from_nested_namespaces") != std::string::npos);
#endif
std::cerr << "cerr 2\n";
    stacktrace ns1 = return_from_nested_namespaces();
    BOOST_TEST(ns1 != return_from_nested_namespaces()); // Different addresses in test_deeply_nested_namespaces() function
    std::cerr << "cerr 3\n";
}

void test_nested() {
std::cerr << "cerr 4\n";
    std::pair<stacktrace, stacktrace> res = foo2(15);
std::cerr << "cerr 5\n";
    std::stringstream ss1, ss2;
std::cerr << "cerr 6\n";
    ss1 << res.first;
    ss2 << res.second;
    std::cout << "'" << ss1.str() << "'\n\n" << ss2.str() << std::endl;
    BOOST_TEST(!ss1.str().empty());
    BOOST_TEST(!ss2.str().empty());
std::cerr << "cerr 7\n";
    BOOST_TEST(ss1.str().find(" 0# ") != std::string::npos);
    BOOST_TEST(ss2.str().find(" 0# ") != std::string::npos);

    BOOST_TEST(ss1.str().find(" 1# ") != std::string::npos);
    BOOST_TEST(ss2.str().find(" 1# ") != std::string::npos);

    BOOST_TEST(ss1.str().find("main") != std::string::npos);
    BOOST_TEST(ss2.str().find("main") != std::string::npos);

#if defined(BOOST_STACKTRACE_DYN_LINK) || !defined(BOOST_STACKTRACE_USE_BACKTRACE)
    BOOST_TEST(ss1.str().find("foo1") != std::string::npos);
    BOOST_TEST(ss1.str().find("foo2") != std::string::npos);
    BOOST_TEST(ss2.str().find("foo1") != std::string::npos);
    BOOST_TEST(ss2.str().find("foo2") != std::string::npos);
#endif
}

void test_comparisons() {
    stacktrace nst = return_from_nested_namespaces();
    stacktrace st;
    stacktrace cst(st);

    BOOST_TEST(nst);
    BOOST_TEST(st);

    BOOST_TEST(nst != st);
    BOOST_TEST(st != nst);
    BOOST_TEST(st == st);
    BOOST_TEST(nst == nst);

    BOOST_TEST(nst != cst);
    BOOST_TEST(cst != nst);
    BOOST_TEST(cst == st);
    BOOST_TEST(cst == cst);

    BOOST_TEST(nst < st || nst > st);
    BOOST_TEST(st < nst || nst < st);
    BOOST_TEST(st <= st);
    BOOST_TEST(nst <= nst);
    BOOST_TEST(st >= st);
    BOOST_TEST(nst >= nst);

    BOOST_TEST(nst < cst || cst < nst);
    BOOST_TEST(nst > cst || cst > nst);


    BOOST_TEST(hash_value(nst) == hash_value(nst));
    BOOST_TEST(hash_value(cst) == hash_value(st));
    
    BOOST_TEST(hash_value(nst) != hash_value(cst));
    BOOST_TEST(hash_value(st) != hash_value(nst));
}

void test_iterators() {
    stacktrace nst = return_from_nested_namespaces();
    stacktrace st;

    BOOST_TEST(nst.begin() != st.begin());
    BOOST_TEST(nst.cbegin() != st.cbegin());
    BOOST_TEST(nst.crbegin() != st.crbegin());
    BOOST_TEST(nst.rbegin() != st.rbegin());

    BOOST_TEST(st.begin() == st.begin());
    BOOST_TEST(st.cbegin() == st.cbegin());
    BOOST_TEST(st.crbegin() == st.crbegin());
    BOOST_TEST(st.rbegin() == st.rbegin());

    BOOST_TEST(++st.begin() == ++st.begin());
    BOOST_TEST(++st.cbegin() == ++st.cbegin());
    BOOST_TEST(++st.crbegin() == ++st.crbegin());
    BOOST_TEST(++st.rbegin() == ++st.rbegin());

    BOOST_TEST(st.begin() + 1 == st.begin() + 1);
    BOOST_TEST(st.cbegin() + 1 == st.cbegin() + 1);
    BOOST_TEST(st.crbegin() + 1 == st.crbegin() + 1);
    BOOST_TEST(st.rbegin() + 1 == st.rbegin() + 1);

    BOOST_TEST(nst.end() != st.end());
    BOOST_TEST(nst.cend() != st.cend());
    BOOST_TEST(nst.crend() != st.crend());
    BOOST_TEST(nst.rend() != st.rend());

    BOOST_TEST(st.end() == st.end());
    BOOST_TEST(st.cend() == st.cend());
    BOOST_TEST(st.crend() == st.crend());
    BOOST_TEST(st.rend() == st.rend());

    BOOST_TEST(--st.end() == --st.end());
    BOOST_TEST(--st.cend() == --st.cend());
    BOOST_TEST(--st.crend() == --st.crend());
    BOOST_TEST(--st.rend() == --st.rend());


    BOOST_TEST(st.end() > st.begin());
    BOOST_TEST(st.end() > st.cbegin());
    BOOST_TEST(st.cend() > st.cbegin());
    BOOST_TEST(st.cend() > st.begin());


    BOOST_TEST(st.size() == static_cast<std::size_t>(st.end() - st.begin()));
    BOOST_TEST(st.size() == static_cast<std::size_t>(st.end() - st.cbegin()));
    BOOST_TEST(st.size() == static_cast<std::size_t>(st.cend() - st.cbegin()));
    BOOST_TEST(st.size() == static_cast<std::size_t>(st.cend() - st.begin()));

    BOOST_TEST(st.size() == static_cast<std::size_t>(std::distance(st.rend(), st.rbegin())));
    BOOST_TEST(st.size() == static_cast<std::size_t>(std::distance(st.rend(), st.crbegin())));
    BOOST_TEST(st.size() == static_cast<std::size_t>(std::distance(st.crend(), st.crbegin())));
    BOOST_TEST(st.size() == static_cast<std::size_t>(std::distance(st.crend(), st.rbegin())));


    boost::stacktrace::stacktrace::iterator it = st.begin();
    ++ it;
    BOOST_TEST(it == st.begin() + 1);
}

void test_frame_view() {
    stacktrace nst = return_from_nested_namespaces();
    stacktrace st;

    const std::size_t min_size = (nst.size() < st.size() ? nst.size() : st.size());
    BOOST_TEST(min_size > 2);

    for (std::size_t i = 0; i < min_size; ++i) {
        BOOST_TEST(st[i] == st[i]);
        BOOST_TEST(st[i].source_file() == st[i].source_file());
        BOOST_TEST(st[i].source_line() == st[i].source_line());
        BOOST_TEST(st[i] <= st[i]);
        BOOST_TEST(st[i] >= st[i]);

        frame_view fv = nst[2];
        if (i >= 2 && i < min_size - 3) { // Begin and end of the trace may match, skipping them
            BOOST_TEST(st[i].name() != fv.name());
            BOOST_TEST(st[i] != fv);
            BOOST_TEST(st[i] < fv || st[i] > fv);
            BOOST_TEST(hash_value(st[i]) != hash_value(fv));
            BOOST_TEST(st[i].source_line() == 0 || st[i].source_file() != fv.source_file());
        }

        fv = st[i];
        BOOST_TEST(hash_value(st[i]) == hash_value(fv));
    }
}

int main() {
    test_deeply_nested_namespaces();
    test_nested();
    test_comparisons();
    test_iterators();
    test_frame_view();

    return boost::report_errors();
}
