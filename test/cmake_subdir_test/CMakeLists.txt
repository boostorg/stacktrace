# Copyright 2018-2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.20)

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
static_assert
throw_exception
optional
type_traits

)

foreach(dep IN LISTS deps)

  add_subdirectory(../../../${dep} boostorg/${dep})

endforeach()

enable_testing()
add_subdirectory(../.. boostorg/stacktrace)

add_executable(main main.cpp)
target_link_libraries(main Boost::stacktrace)
set_property(TARGET main PROPERTY CXX_SCAN_FOR_MODULES ON)

add_test(main main)

if (BOOST_USE_MODULES)
    add_executable(boost_stacktrace_module_usage ../../modules/usage_sample.cpp)
    set_property(TARGET boost_stacktrace_module_usage PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_link_libraries(boost_stacktrace_module_usage PRIVATE Boost::stacktrace)
    add_test(NAME boost_stacktrace_module_usage COMMAND boost_stacktrace_module_usage)
endif()
