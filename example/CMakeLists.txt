# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set(sources 
    terminate_handler.cpp
    ../CMakeLists.txt
)

organizeFiles(${sources})

add_executable(terminate_handler ${sources})

target_include_directories(terminate_handler PRIVATE ../../..)
set_target_properties(terminate_handler PROPERTIES FOLDER stacktrace/example)

target_compile_definitions(terminate_handler PRIVATE BOOST_ALL_NO_LIB)

target_link_libraries( terminate_handler PUBLIC
    boost_date_time
    boost_filesystem
)

# C Sharp example test application
set(CMAKE_CSharp_FLAGS "/define:TRACE /langversion:7.3 /platform:x64")

include(CSharpUtilities)

set(wpf_exe_srcs
    csharp_crashy_app.cs
    csharp_crashy_window.xaml
    csharp_crashy_window.xaml.cs
)
add_executable(csharp_wpf_exe ${wpf_exe_srcs})
set_target_properties(csharp_wpf_exe PROPERTIES FOLDER stacktrace/example)

set_target_properties( csharp_wpf_exe PROPERTIES
    VS_GLOBAL_ROOTNAMESPACE "csharp_wpf_exe"
    VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
    VS_DOTNET_REFERENCES "System;System.Data;System.XML;Microsoft.CSharp;WindowsBase;PresentationCore;PresentationFramework;System.ComponentModel.Composition;System.Xaml"
)
csharp_set_xaml_cs_properties(${wpf_exe_srcs})

