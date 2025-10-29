# Copyright 2018-2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...4.20)

project(cmake_install_test LANGUAGES CXX)

find_package(
    Boost "$ENV{BOOST_SUPERPROJECT_VERSION}"
    REQUIRED ${BOOST_STACKTRACE_IMPL_BACKEND} stacktrace_from_exception
    CONFIG
)
add_executable(main main.cpp)
target_link_libraries(main Boost::${BOOST_STACKTRACE_IMPL_BACKEND})

add_executable(main_from_exception main.cpp)
target_link_libraries(main_from_exception Boost::stacktrace_from_exception Boost::${BOOST_STACKTRACE_IMPL_BACKEND})

enable_testing()
add_test(main main)
add_test(main_from_exception main_from_exception)
