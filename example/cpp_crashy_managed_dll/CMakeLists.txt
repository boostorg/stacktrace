# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
include(boost_helpers)

# These variables affects also other C++ projects, that's why separate cmake for this c++ dll

# Clr compiling requires these:

# "Enable C++ Exceptions" - "Yes (/EHsc)" => "Yes with SEH Exceptions (/EHa)"
string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# "Basic Run-time checks" - "Both (/RTC1, equiv. to /RTCsu) (/RTC1)" => "Default"
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

# C++ crashy dll
set(srcs cpp_crashy_managed_dll.cpp)
add_library(cpp_crashy_managed_dll SHARED ${srcs})
set_target_properties(cpp_crashy_managed_dll PROPERTIES FOLDER stacktrace/example)
organizeFiles(${srcs})
set_target_properties(cpp_crashy_managed_dll PROPERTIES VS_GLOBAL_CLRSupport "true")
set_property(TARGET cpp_crashy_managed_dll PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
target_include_directories(cpp_crashy_managed_dll PRIVATE ../../../..)
boost_project(cpp_crashy_managed_dll)

