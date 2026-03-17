# Copyright 2018-2020 Peter Dimov
# Copyright 2026 Fedor Osetrov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

if(NOT TARGET tests)
  add_custom_target(tests)
endif()

function(add_stacktrace_test name libs sources)
  set(test_name stacktrace_${name})

  add_executable(${test_name} ${name}.cpp ${sources})
  target_link_libraries(${test_name} PRIVATE ${libs})
  add_test(NAME ${test_name} COMMAND ${test_name})
  add_dependencies(tests ${test_name})
endfunction()

add_stacktrace_test(test_num_conv Boost::stacktrace "")
add_stacktrace_test(test_void_ptr_cast Boost::stacktrace "")
add_stacktrace_test(test_noop Boost::stacktrace_noop test_impl.cpp)
add_stacktrace_test(test_trivial Boost::stacktrace test_impl.cpp)
add_stacktrace_test(test Boost::stacktrace test_impl.cpp)
add_stacktrace_test(test_thread_safety_checking "Boost::stacktrace;Boost::optional" test_impl.cpp)
add_stacktrace_test(test_from_exception_none Boost::stacktrace "")
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_stacktrace_test(test_from_exception "Boost::stacktrace;Boost::stacktrace_from_exception" "")
endif()
if(NOT __default_stacktrace_backend STREQUAL "addr2line")
  add_stacktrace_test(test_torture Boost::stacktrace test_impl.cpp)
endif()
