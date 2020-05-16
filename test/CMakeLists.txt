# Copyright 2020 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(boost_helpers)

set(libs boost_stacktrace boost_stacktrace_windbg)

set(boost_stacktrace_DEFINES BOOST_STACKTRACE)
set(boost_stacktrace_windbg_DEFINES BOOST_STACKTRACE_DYN_LINK)

file(GLOB examples ../example/*.cpp)
foreach(p ${examples})
    get_filename_component(target_name ${p} NAME_WE)
    foreach(l ${libs})
        set(project example_${target_name}_${l})

        if(${target_name} STREQUAL cpp_crashy_app)
            continue()
        endif()

        add_executable(${project} ${p})
        target_include_directories(${project} PRIVATE ../../..)
        target_compile_definitions(${project} PRIVATE ${${l}_DEFINES})
        set_target_properties(${project} PROPERTIES FOLDER stacktrace/tests/example)
        target_link_libraries(${project} PRIVATE ${l})
        boost_project(${project})
    endforeach()
endforeach()


foreach(l ${libs})
    set(project test_${l})
    add_executable(${project} test.cpp test_impl.cpp)
    target_include_directories(${project} PRIVATE ../../..)
    target_compile_definitions(${project} PRIVATE ${${l}_DEFINES})
    set_target_properties(${project} PROPERTIES FOLDER stacktrace/tests/test)
    target_link_libraries(${project} PRIVATE ${l})
    boost_project(${project})

    set(project torture_${l})
    add_executable(${project} torture.cpp test_impl.cpp)
    target_include_directories(${project} PRIVATE ../../..)
    target_compile_definitions(${project} PRIVATE ${${l}_DEFINES})
    set_target_properties(${project} PROPERTIES FOLDER stacktrace/tests/torture)
    target_link_libraries(${project} PRIVATE ${l})
    boost_project(${project})
endforeach()

