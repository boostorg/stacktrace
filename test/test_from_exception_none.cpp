// Copyright Antony Polukhin, 2023-2024.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stacktrace.hpp>

#include <boost/core/lightweight_test.hpp>

using boost::stacktrace::current_exception_stacktrace;
using boost::stacktrace::stacktrace;


BOOST_NOINLINE BOOST_SYMBOL_VISIBLE void in_test_throw_1(const char* msg) {
  std::string new_msg{msg};
  throw std::runtime_error(new_msg);
}

BOOST_NOINLINE BOOST_SYMBOL_VISIBLE void test_no_trace_from_exception() {
  try {
    in_test_throw_1("testing basic");
  } catch (const std::exception&) {
    auto trace = current_exception_stacktrace();
    BOOST_TEST(!trace);
  }
}

int main() {
  test_no_trace_from_exception(); 

  return boost::report_errors();
}
