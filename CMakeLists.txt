# Copyright 2018 Tarmo Pikaro
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.15)

# Reorganize projects into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_language(CSharp)
enable_language(CXX)
enable_language(C)

project(stacktrace)

#set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs )
#set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
#set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )

# Organizes files as in file system within visual studio project.
macro(organizeFiles)
    # Reorganize files into folders in project
    foreach(relPath IN ITEMS ${ARGN})
        get_filename_component(dir "${relPath}" DIRECTORY)
        # source_group uses '\\' , fs '/' as group separator
        string(REPLACE "/" "\\" dir "${dir}")
        # collapse extra folder complexity away - no '..'
        string(REPLACE "..\\" "" dir "${dir}")
        # shorten path, include\boost\ => boost\ 
        string(REGEX REPLACE "^include\\\\boost" "boost" dir "${dir}")
        source_group("${dir}" FILES "${relPath}")
    endforeach()
endmacro()

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(test)

