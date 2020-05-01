# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(boost_helpers)


# C++ crashy exe
set(srcs 
    cpp_crashy_app.cpp
    ../CMakeLists.txt
    ../cmake/boost_helpers.cmake
    ../Build/Jamfile.v2
)

organizeFiles(${srcs})

add_executable(cpp_crashy_app ${srcs})
boost_project(cpp_crashy_app)

target_include_directories(cpp_crashy_app PRIVATE ../../..)
set_target_properties(cpp_crashy_app PROPERTIES FOLDER stacktrace/example)

target_compile_definitions(cpp_crashy_app PRIVATE BOOST_ALL_NO_LIB;BOOST_STACKTRACE)

target_link_libraries( cpp_crashy_app PUBLIC
    boost_date_time
    boost_filesystem
    boost_stacktrace
)

# C++ crashy dll
add_subdirectory(cpp_crashy_managed_dll)
boost_project(cpp_crashy_managed_dll)

# C Sharp example test application
set(CMAKE_CSharp_FLAGS "/define:TRACE /langversion:7.3")

if(${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
    set(CMAKE_CSharp_FLAGS "${CMAKE_CSharp_FLAGS} /platform:x64")
endif()

include(CSharpUtilities)

set(wpf_exe_srcs
    csharp_crashy_app.cs
    csharp_crashy_window.xaml
    csharp_crashy_window.xaml.cs
)
add_executable(csharp_crashy_app ${wpf_exe_srcs})
set_target_properties(csharp_crashy_app PROPERTIES FOLDER stacktrace/example)

set_target_properties( csharp_crashy_app PROPERTIES
    VS_GLOBAL_ROOTNAMESPACE "csharp_crashy_app"
    VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
    VS_DOTNET_REFERENCES "System;System.Data;System.XML;Microsoft.CSharp;WindowsBase;PresentationCore;PresentationFramework;System.ComponentModel.Composition;System.Xaml"
)
csharp_set_xaml_cs_properties(${wpf_exe_srcs})

add_dependencies(csharp_crashy_app cpp_crashy_managed_dll)

