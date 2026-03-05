# Copyright 2018-2020 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

if(NOT TARGET tests)
  add_custom_target(tests)
endif()

function(add_stacktrace_test name libs sources)
  set(test_name stacktrace_test${name})

  add_executable(${test_name} test${name}.cpp ${sources})
  target_link_libraries(${test_name} PRIVATE ${libs})
  add_test(NAME ${test_name} COMMAND ${test_name})
  add_dependencies(tests ${test_name})
endfunction()

add_stacktrace_test(_num_conv Boost::stacktrace "")
add_stacktrace_test(_void_ptr_cast Boost::stacktrace "")
add_stacktrace_test(_noop Boost::stacktrace_noop test_impl.cpp)
add_stacktrace_test(_trivial Boost::stacktrace test_impl.cpp)
add_stacktrace_test("" Boost::stacktrace test_impl.cpp)
add_stacktrace_test(_thread_safety_checking "Boost::stacktrace;Boost::optional" test_impl.cpp)
add_stacktrace_test(_from_exception "Boost::stacktrace;Boost::stacktrace_from_exception" "")
add_stacktrace_test(_from_exception_none Boost::stacktrace "")
add_stacktrace_test(_torture Boost::stacktrace test_impl.cpp)
