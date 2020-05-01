# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(boost_helpers)

set(stacktraceIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Just in case if we have stripped repository, boost/include is official place for boost includes
if(NOT EXISTS ${stacktraceIncludeDir})
    set(stacktraceIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/../../../boost/stacktrace)
endif()

file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    ${stacktraceIncludeDir}/*.hpp 
    ${stacktraceIncludeDir}/*.h 
    ${stacktraceIncludeDir}/*.ipp
)

set(sources windbg.cpp ${headers} ../Build/Jamfile.v2)

organizeFiles(${sources})

add_library(boost_stacktrace_windbg SHARED ${sources})
target_include_directories(boost_stacktrace_windbg PRIVATE ../../..)
target_compile_definitions(boost_stacktrace_windbg PRIVATE BOOST_STACKTRACE_DYN_LINK=1)
#target_compile_definitions(boost_stacktrace_windbg PRIVATE BOOST_STACKTRACE_DYN_LINK=1)
set_target_properties(boost_stacktrace_windbg PROPERTIES FOLDER stacktrace/src)


set(sources 
    ${headers}
    ../Build/Jamfile.v2
    exception_handler.cpp
    minhook/src/buffer.c
    minhook/src/hook.c
    minhook/src/trampoline.c
    minhook/src/hde/hde64.c
    minhook/src/hde/hde32.c

    minhook/src/buffer.h
    minhook/src/hde/hde32.h
    minhook/src/hde/hde64.h
    minhook/include/MinHook.h
    minhook/src/hde/pstdint.h
    minhook/src/hde/table32.h
    minhook/src/hde/table64.h
    minhook/src/trampoline.h
    minhook/LICENSE.txt
)

organizeFiles(${sources})

add_library(boost_stacktrace STATIC ${sources})
target_include_directories(boost_stacktrace PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(boost_stacktrace PRIVATE ../../..)
target_compile_definitions(boost_stacktrace PRIVATE BOOST_STACKTRACE=1)
set_target_properties(boost_stacktrace PROPERTIES FOLDER stacktrace/src)

boost_project(boost_stacktrace)

