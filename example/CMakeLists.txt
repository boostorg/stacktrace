# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

add_executable(terminate_handler terminate_handler.cpp)

target_include_directories(terminate_handler PRIVATE ../../..)
set_target_properties(terminate_handler PROPERTIES FOLDER stacktrace/example)

target_compile_definitions(terminate_handler PRIVATE BOOST_ALL_NO_LIB)

target_link_libraries( terminate_handler PUBLIC
    boost_date_time
    boost_filesystem
)

