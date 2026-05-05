# Copyright 2018-2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...4.20)

project(cmake_subdir_test LANGUAGES CXX)

# Put boost_stacktrace_*.dll in the same directory as main.exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# boostdep --brief stacktrace

set(deps

# Primary dependencies

assert
config
container_hash
core
predef
winapi

# Secondary dependencies

describe
mp11
throw_exception
optional
type_traits

)

foreach(dep IN LISTS deps)

  add_subdirectory(../../../${dep} boostorg/${dep})

endforeach()

enable_testing()
add_subdirectory(../.. boostorg/stacktrace)

add_executable(stacktrace_main main.cpp)
target_link_libraries(stacktrace_main Boost::stacktrace)

add_executable(stacktrace_main_header_only main.cpp)
target_include_directories(stacktrace_main_header_only PRIVATE ../../include)
target_compile_definitions(stacktrace_main_header_only PRIVATE "_GNU_SOURCE=1")
target_link_libraries(stacktrace_main_header_only PRIVATE Boost::core Boost::container_hash Boost::predef Boost::winapi)

add_test(NAME stacktrace_main COMMAND stacktrace_main)
add_test(NAME stacktrace_main_header_only COMMAND stacktrace_main_header_only)

if (BOOST_USE_MODULES)
    add_executable(boost_stacktrace_module_usage ../../modules/samples/usage_sample.cpp)
    target_link_libraries(boost_stacktrace_module_usage PRIVATE Boost::stacktrace)
    add_test(NAME boost_stacktrace_module_usage COMMAND boost_stacktrace_module_usage)

    add_executable(boost_stacktrace_from_exception_module_usage ../../modules/samples/usage_sample_from_exception.cpp)
    target_link_libraries(boost_stacktrace_from_exception_module_usage PRIVATE Boost::stacktrace Boost::stacktrace_from_exception)
    add_test(NAME boost_stacktrace_from_exception_module_usage COMMAND boost_stacktrace_from_exception_module_usage)

    foreach(backend noop basic addr2line backtrace)
      if (NOT TARGET Boost::stacktrace_${backend})
        message(STATUS "Skipping ${backend} backend")
        continue()
      endif()
      add_executable(boost_stacktrace_module_usage_${backend} ../../modules/samples/usage_sample.cpp)
      target_link_libraries(boost_stacktrace_module_usage_${backend} PRIVATE Boost::stacktrace_${backend})
      target_compile_definitions(boost_stacktrace_module_usage_${backend} PRIVATE STACKTRACE_MODULE=boost.stacktrace.${backend})
      add_test(NAME boost_stacktrace_module_usage_${backend} COMMAND boost_stacktrace_module_usage_${backend})

      add_executable(boost_stacktrace_from_exception_module_usage_${backend} ../../modules/samples/usage_sample_from_exception.cpp)
      target_link_libraries(boost_stacktrace_from_exception_module_usage_${backend} PRIVATE Boost::stacktrace_${backend} Boost::stacktrace_from_exception)
      target_compile_definitions(boost_stacktrace_from_exception_module_usage_${backend} PRIVATE STACKTRACE_MODULE=boost.stacktrace.${backend})
      add_test(NAME boost_stacktrace_from_exception_module_usage_${backend} COMMAND boost_stacktrace_from_exception_module_usage_${backend})
    endforeach()
endif()
