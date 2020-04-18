cmake_minimum_required(VERSION 3.15)

project(stacktrace)

set(stacktraceIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Just in case if we have stripped repository, boost/include is official place for boost includes
if(NOT EXISTS ${stacktraceIncludeDir})
    set(stacktraceIncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/../../boost/stacktrace)
endif()

file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    ${stacktraceIncludeDir}/*.hpp 
    ${stacktraceIncludeDir}/*.ipp
)

set(sources src/windbg.cpp ${headers})

add_library(boost_stacktrace_windbg SHARED ${sources})
target_include_directories(boost_stacktrace_windbg PRIVATE ../..)
target_compile_definitions(boost_stacktrace_windbg PRIVATE BOOST_STACKTRACE_DYN_LINK=1)

# Reorganize files into folders in project
foreach(relPath IN LISTS sources)
    get_filename_component(dir "${relPath}" DIRECTORY)
    # source_group uses '\\' , fs '/' as group separator
    string(REPLACE "/" "\\" dir "${dir}")
    # collapse extra folder complexity away - no '..'
    string(REPLACE "..\\" "" dir "${dir}")
    # boost\ => include\
    string(REPLACE "boost\\" "include\\" dir "${dir}")
    if(NOT ${dir} STREQUAL "")
        source_group("${dir}" FILES "${relPath}")
    endif()
endforeach()

