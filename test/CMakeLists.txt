# Copyright 2020 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(boost_helpers)
include(CSharpUtilities)    #csharp_set_xaml_cs_properties

set(CMAKE_CSharp_FLAGS "/define:TRACE /langversion:7.3")

#set(libs boost_stacktrace boost_stacktrace_windbg)
set(libs boost_stacktrace)

set(boost_stacktrace_DEFINES BOOST_STACKTRACE)
set(boost_stacktrace_windbg_DEFINES BOOST_STACKTRACE_DYN_LINK)

macro(querySubstring keyvar var)
    set(is_${keyvar} False)
    string(FIND "${var}" ${keyvar} pos)
    if(NOT ${pos} EQUAL -1)
        set(is_${keyvar} True)
    endif()
endmacro()

function (add_project project projectFolder)
    cmake_parse_arguments(PROJ "" "" "SRCS;DEFINES;LINKS;DEPENDS;LINK_FLAGS;SHARP_COMPILE_FLAGS" ${ARGN} )

    querySubstring(managed ${PROJ_SRCS} )
    querySubstring(dll ${PROJ_SRCS} )
    querySubstring(csharp ${project} )

    if(${is_dll})
        add_library(${project} SHARED ${PROJ_SRCS})
    else()
        add_executable(${project} ${PROJ_SRCS})
    endif()

    if(${is_csharp})
        set(is_managed True)
        csharp_set_xaml_cs_properties(${PROJ_SRCS})
        set_target_properties( ${project} PROPERTIES
            VS_DOTNET_REFERENCES "System;System.Data;System.XML;Microsoft.CSharp;WindowsBase;PresentationCore;PresentationFramework;System.ComponentModel.Composition;System.Xaml"
        )
    endif()

    if(${is_csharp})
        set_property(TARGET ${project} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
        set(compile_flags ${PROJ_SHARP_COMPILE_FLAGS})
    else()
        if(${is_managed})
            # Managed C++ code
            set_target_properties(${project} PROPERTIES VS_GLOBAL_CLRSupport "true")
            set_property(TARGET ${project} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")

            # Being able to debug dll
            target_link_options(${project} PRIVATE "$<$<CONFIG:Debug>:/ASSEMBLYDEBUG>")
        
            # Disable incremental linking
            target_link_options(${project} PRIVATE "$<$<CONFIG:Debug>:/INCREMENTAL:NO>")

            # "Enable C++ Exceptions" - "Yes with SEH Exceptions (/EHa)"
            set(compile_flags /EHa)
        else()
            # Native C++ code
            # "Basic Run-time checks" - "Both (/RTC1, equiv. to /RTCsu) (/RTC1)" => "Default"
            target_compile_options(${project} PRIVATE "$<$<CONFIG:Debug>:/RTC1>")

            # Enable incremental linking
            target_link_options(${project} PRIVATE "$<$<CONFIG:Debug>:/INCREMENTAL>")

            # "Enable C++ Exceptions" - "Yes (/EHsc)"
            set(compile_flags /EHsc)
        endif()
    endif()
    set_target_properties(${project} PROPERTIES COMPILE_FLAGS ${compile_flags})

    target_include_directories(${project} PRIVATE ../../..)
    target_compile_definitions(${project} PRIVATE ${PROJ_DEFINES})
    set_target_properties(${project} PROPERTIES FOLDER ${projectFolder})
    target_link_libraries(${project} PRIVATE ${PROJ_LINKS})
    boost_project(${project})

    if (MSVC)
        set(opt /LIBPATH:)
    else()
        set(opt -L)
    endif ()
    target_link_options(${project} PRIVATE ${opt}${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>)

    foreach(d ${PROJ_DEPENDS})
        if(TARGET ${d})
            if(TARGET ${d})
                target_link_libraries(${project} PRIVATE ${d})
            else()
                add_dependencies(${project} ${d})
            endif()
        endif()
    endforeach()

    organizeFiles(${PROJ_SRCS})
    if(NOT ${PROJ_LINK_FLAGS} STREQUAL "")
        set_target_properties(${project} PROPERTIES LINK_FLAGS ${PROJ_LINK_FLAGS})
    endif()
endfunction()

function (add_external_project project opposite_platform projectFolder)
    set(ext ${ARGN})
    if(NOT DEFINED ext)
        set(ext vcxproj)
    endif()

    set(file ${CMAKE_CURRENT_BINARY_DIR}/${project}.${ext})
    if(${opposite_platform} STREQUAL "Win32")
        set(opposite_platform_short "32")
    else()
        set(opposite_platform_short "64")
    endif()
    string(REPLACE "/${current_platform}/" "/${opposite_platform_short}/" file ${file})

    if(EXISTS ${file})
        include_external_msproject(${project} ${file} PLATFORM ${opposite_platform})
        set_target_properties(${project} PROPERTIES FOLDER ${projectFolder} )
    else()
        if(NOT ${ext} STREQUAL csproj)
            add_external_project(${project} ${opposite_platform} ${projectFolder} csproj)
        endif()
    endif()
endfunction()



set(VAR_LIST 32;64)
set(VAR_32_LINK_FLAGS /MACHINE:X86)
set(VAR_64_LINK_FLAGS /MACHINE:X64)
set(VAR_32_SHARP_COMPILE_FLAGS /platform:x86)
set(VAR_64_SHARP_COMPILE_FLAGS /platform:x64)
#foreach(p ${VAR_LIST})
#    message("${p} ${VAR_${p}_LINK_FLAGS}")
#endforeach()

get_current_platform()

if(${current_platform} STREQUAL "64")
    set(opposite_platform "Win32")
else()
    set(opposite_platform "x64")
endif()


set(TESTS ON)
if(NOT ${BUILD_TESTING})
    set(TESTS OFF)
endif()

option(TEST_STACKTRACE "Enable all tests for stacktrace" ${TESTS})
option(TEST_STACKTRACE_MANAGED "Enable managed tests for stacktrace" ${TESTS})


file(GLOB_RECURSE examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../example/*.cpp)
foreach(p ${examples})
    get_filename_component(target_name ${p} NAME_WE)
    foreach(l ${libs})
        set(proj_depends)

        if(${target_name} STREQUAL cpp_crashy_app)
            set(proj_depends boost_filesystem boost_date_time)
        endif()

        #message(${target_name})

        if(NOT ${TEST_STACKTRACE})
            if(${TEST_STACKTRACE_MANAGED} AND NOT (${target_name} STREQUAL cpp_crashy_managed_dll ))
                continue()
            endif()
        endif()

        foreach(v ${VAR_LIST})

            if(NOT ${v} STREQUAL ${current_platform})
                if(NOT ${target_name} STREQUAL cpp_crashy_app)
                    add_external_project(example_${target_name}_${l}_${v} ${opposite_platform} stacktrace/example)
                endif()
                continue()
            endif()

            add_project(example_${target_name}_${l}_${v} stacktrace/example 
                SRCS ${p} 
                DEFINES ${${l}_DEFINES} 
                LINKS ${l}_${current_platform} 
                DEPENDS ${proj_depends}
                LINK_FLAGS ${VAR_${v}_LINK_FLAGS}
            )
        endforeach()
    endforeach()
endforeach()

# --------------------------- C# examples ---------------------------
file(GLOB cs_examples ../example/*.cs ../example/*.xaml)
foreach(p ${cs_examples})
    #csharp_crashy_app.cs => project "csharp_crashy" with that file.
    string(FIND ${p} "_" pos REVERSE)
    string(SUBSTRING ${p} 0 ${pos} out)
    get_filename_component(projName ${out} NAME_WE)
    set(${projName}_SRC ${p} ${${projName}_SRC})
    if(NOT ${projName} IN_LIST cs_projects)
        list(APPEND cs_projects ${projName})
    endif()
endforeach()

foreach(p ${cs_projects})

    foreach(v ${VAR_LIST})

        if(NOT ${v} STREQUAL ${current_platform})
            add_external_project(example_${p}_${v} ${opposite_platform} stacktrace/example)
            continue()
        endif()

        add_project(example_${p}_${v} stacktrace/example 
            SRCS ${${p}_SRC} 
            DEPENDS example_cpp_crashy_managed_dll_boost_stacktrace_${v}
            SHARP_COMPILE_FLAGS ${VAR_${v}_SHARP_COMPILE_FLAGS}
        )

    endforeach()
endforeach()

foreach(v ${VAR_LIST})
    foreach(l ${libs})
        set(project test_${l}_${v})
        
        if(NOT ${TEST_STACKTRACE})
            continue()
        endif()

        if(NOT ${v} STREQUAL ${current_platform})
            add_external_project(${project} ${opposite_platform} stacktrace/test)
            add_external_project(torture_${l}_${v} ${opposite_platform} stacktrace/torture)
            continue()
        endif()

        add_project(${project} stacktrace/test 
            SRCS test.cpp test_impl.cpp 
            DEFINES ${${l}_DEFINES} 
            LINKS ${l}_${current_platform}
            LINK_FLAGS ${VAR_${v}_LINK_FLAGS}
        )

        add_project(torture_${l}_${v} stacktrace/torture 
            SRCS torture.cpp test_impl.cpp 
            DEFINES ${${l}_DEFINES} 
            LINKS ${l}_${current_platform}
            LINK_FLAGS ${VAR_${v}_LINK_FLAGS}
        )

    endforeach()
endforeach()


# Disable global flag, configure on per project basis, using target_compile_options, target_link_options
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "/INCREMENTAL" "" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
#string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

