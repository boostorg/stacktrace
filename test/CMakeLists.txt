# Copyright 2018-2020 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

if(NOT TARGET tests)
  add_custom_target(tests)
endif()

add_executable(stacktrace_test_num_conv
  test_num_conv.cpp)
target_link_libraries(stacktrace_test_num_conv Boost::stacktrace Boost::core)
add_test(NAME stacktrace_test_num_conv COMMAND stacktrace_test_num_conv)
add_dependencies(tests stacktrace_test_num_conv)

add_executable(stacktrace_test_void_ptr_cast
  test_void_ptr_cast.cpp)
target_link_libraries(stacktrace_test_void_ptr_cast Boost::stacktrace Boost::core)
add_test(NAME stacktrace_test_void_ptr_cast COMMAND stacktrace_test_void_ptr_cast)
add_dependencies(tests stacktrace_test_void_ptr_cast)

add_executable(stacktrace_test_trivial
  test_trivial.cpp test_impl.cpp)
target_link_libraries(stacktrace_test_trivial Boost::stacktrace Boost::core)
add_test(NAME stacktrace_test_trivial COMMAND stacktrace_test_trivial)
add_dependencies(tests stacktrace_test_trivial)

# add_executable(stacktrace_test_noop
#   test_noop.cpp test_impl.cpp)
# target_link_libraries(stacktrace_test_noop Boost::stacktrace_noop Boost::core)
# add_test(NAME stacktrace_test_noop COMMAND stacktrace_test_noop)
# add_dependencies(tests stacktrace_test_noop)

add_executable(stacktrace_test
  test.cpp test_impl.cpp)
target_link_libraries(stacktrace_test PRIVATE Boost::stacktrace Boost::core)
add_test(NAME stacktrace_test COMMAND stacktrace_test)
add_dependencies(tests stacktrace_test)

# add_executable(stacktrace_test_from_exception
#   test_from_exception.cpp)
# target_link_libraries(stacktrace_test_from_exception Boost::stacktrace_from_exception Boost::core)
# add_test(NAME stacktrace_test_from_exception COMMAND stacktrace_test_from_exception)
# add_dependencies(tests stacktrace_test_from_exception)

# add_executable(stacktrace_test_from_exception_none
#   test_from_exception_none.cpp)
# target_link_libraries(stacktrace_test_from_exception_none Boost::stacktrace Boost::core)
# add_test(NAME stacktrace_test_from_exception_none COMMAND stacktrace_test_from_exception_none)
# add_dependencies(tests stacktrace_test_from_exception_none)

add_executable(stacktrace_test_thread_safety_checking
  thread_safety_checking.cpp test_impl.cpp)
target_link_libraries(stacktrace_test_thread_safety_checking Boost::stacktrace Boost::core Boost::optional)
add_test(NAME stacktrace_test_thread_safety_checking COMMAND stacktrace_test_thread_safety_checking)
add_dependencies(tests stacktrace_test_thread_safety_checking)

add_executable(stacktrace_test_torture
  torture.cpp test_impl.cpp)
target_link_libraries(stacktrace_test_torture Boost::stacktrace)
add_test(NAME stacktrace_test_torture COMMAND stacktrace_test_torture)
add_dependencies(tests stacktrace_test_torture)
