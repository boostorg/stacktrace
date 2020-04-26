# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

# C++ crashy exe
set(srcs 
    cpp_crashy_app.cpp
    ../CMakeLists.txt
)

organizeFiles(${srcs})

add_executable(cpp_crashy_app ${srcs})

target_include_directories(cpp_crashy_app PRIVATE ../../..)
set_target_properties(cpp_crashy_app PROPERTIES FOLDER stacktrace/example)

target_compile_definitions(cpp_crashy_app PRIVATE BOOST_ALL_NO_LIB)

target_link_libraries( cpp_crashy_app PUBLIC
    boost_date_time
    boost_filesystem
)

# C++ crashy dll
add_subdirectory(cpp_crashy_managed_dll)

# C Sharp example test application
set(CMAKE_CSharp_FLAGS "/define:TRACE /langversion:7.3 /platform:x64")

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

