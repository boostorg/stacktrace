# Copyright 2020 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(boost_helpers)

set(libs boost_stacktrace boost_stacktrace_windbg)

set(boost_stacktrace_DEFINES BOOST_STACKTRACE)
set(boost_stacktrace_windbg_DEFINES BOOST_STACKTRACE_DYN_LINK)

#[[
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
    #message("${p} - ${${p}_SRC}")
endforeach()
]]#

function (add_test_project project projectFolder)
    cmake_parse_arguments(PROJ "" "" "SRCS;DEFINES;LINKS;DEPENDS;LINK_FLAGS" ${ARGN} )
    add_executable(${project} ${PROJ_SRCS})
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
            add_dependencies(${project} ${d})
        endif()
    endforeach()


    organizeFiles(${PROJ_SRCS})
    message("PROJ_LINK_FLAGS: ${PROJ_LINK_FLAGS}")
    if(NOT ${PROJ_LINK_FLAGS} STREQUAL "")
        set_target_properties(${project} PROPERTIES LINK_FLAGS ${PROJ_LINK_FLAGS})
        message("Using ${PROJ_LINK_FLAGS}")
    endif()
endfunction()

set(VAR_LIST 32;64)
set(VAR_32_LINK_FLAGS /MACHINE:X86)
set(VAR_64_LINK_FLAGS /MACHINE:X64)
#foreach(p ${VAR_LIST})
#    message("${p} ${VAR_${p}_LINK_FLAGS}")
#endforeach()

file(GLOB examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../example/*.cpp)
foreach(p ${examples})
    get_filename_component(target_name ${p} NAME_WE)
    foreach(l ${libs})
        set(proj_depends)

        if(${target_name} STREQUAL cpp_crashy_app)
            set(proj_depends boost_filesystem boost_date_time)
        endif()

        foreach(v ${VAR_LIST})
            add_test_project(example_${v}_${target_name}_${l} stacktrace/tests/example 
                SRCS ${p} 
                DEFINES ${${l}_DEFINES} 
                LINKS ${l} DEPENDS ${proj_depends}
                LINK_FLAGS ${VAR_${v}_LINK_FLAGS}
            )
        endforeach()
    endforeach()
endforeach()

foreach(v ${VAR_LIST})
    foreach(l ${libs})

        add_test_project(test_${v}_${l} stacktrace/tests/test 
            SRCS test.cpp test_impl.cpp 
            DEFINES ${${l}_DEFINES} 
            LINKS ${l}
            LINK_FLAGS ${VAR_${v}_LINK_FLAGS}
        )

        add_test_project(torture_${v}_${l} stacktrace/tests/torture 
            SRCS torture.cpp test_impl.cpp 
            DEFINES ${${l}_DEFINES} 
            LINKS ${l}
            LINK_FLAGS ${VAR_${v}_LINK_FLAGS}
        )

    endforeach()
endforeach()

