# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set(stacktraceIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Just in case if we have stripped repository, boost/include is official place for boost includes
if(NOT EXISTS ${stacktraceIncludeDir})
    set(stacktraceIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/../../../boost/stacktrace)
endif()

file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    ${stacktraceIncludeDir}/*.hpp 
    ${stacktraceIncludeDir}/*.ipp
)

set(sources windbg.cpp ${headers} ../Build/Jamfile.v2)

organizeFiles(${sources})

add_library(boost_stacktrace_windbg SHARED ${sources})
target_include_directories(boost_stacktrace_windbg PRIVATE ../../..)
target_compile_definitions(boost_stacktrace_windbg PRIVATE BOOST_STACKTRACE_DYN_LINK=1)
#target_compile_definitions(boost_stacktrace_windbg PRIVATE BOOST_STACKTRACE_DYN_LINK=1)
set_target_properties(boost_stacktrace_windbg PROPERTIES FOLDER stacktrace_libs)

