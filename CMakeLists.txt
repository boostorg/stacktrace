# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.8...4.20)

project(boost_stacktrace VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

function(stacktrace_add_library suffix opt libs defs add_module)

  if(NOT opt)
    return()
  endif()

  add_library(boost_stacktrace_${suffix})

  add_library(Boost::stacktrace_${suffix} ALIAS boost_stacktrace_${suffix})

  target_include_directories(boost_stacktrace_${suffix} PUBLIC include)

  target_link_libraries(boost_stacktrace_${suffix}
    PUBLIC
      Boost::config
      Boost::container_hash
      Boost::core
      Boost::predef
      Boost::winapi
    PRIVATE
      ${libs}
  )

  target_compile_definitions(boost_stacktrace_${suffix}
    PUBLIC
      BOOST_STACKTRACE_NO_LIB
    PRIVATE
      BOOST_STACKTRACE_SOURCE
      ${defs}
  )

  target_sources(boost_stacktrace_${suffix}
    PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/src/${suffix}.cpp
  )
  if (add_module)
    target_sources(boost_stacktrace_${suffix}
      PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/modules
        FILES ${CMAKE_CURRENT_LIST_DIR}/modules/backends/boost_stacktrace_${suffix}.cppm
    )
    target_compile_features(boost_stacktrace_${suffix} PUBLIC cxx_std_23)
    target_compile_definitions(boost_stacktrace_${suffix}
      PUBLIC
        BOOST_USE_MODULES
      INTERFACE
        BOOST_STACKTRACE_BACKEND_MODULE=${suffix}
    )
  endif()


  if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_stacktrace_${suffix} PUBLIC BOOST_STACKTRACE_DYN_LINK)
  else()
    target_compile_definitions(boost_stacktrace_${suffix} PUBLIC BOOST_STACKTRACE_STATIC_LINK)
  endif()

  if(BOOST_SUPERPROJECT_VERSION AND NOT CMAKE_VERSION VERSION_LESS 3.13)
    boost_install(TARGETS boost_stacktrace_${suffix} VERSION ${BOOST_SUPERPROJECT_VERSION} HEADER_DIRECTORY include)
  endif()

endfunction()

include(CheckCXXSourceCompiles)

function(stacktrace_check var source incs libs)

  set(CMAKE_REQUIRED_INCLUDES "${incs}")
  list(APPEND CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/build")
  set(CMAKE_REQUIRED_LIBRARIES "${libs}")
  check_cxx_source_compiles("#include \"${source}\"" ${var})
  set(${var} ${${var}} PARENT_SCOPE)

endfunction()

stacktrace_check(BOOST_STACKTRACE_HAS_BACKTRACE has_backtrace.cpp "" "backtrace")

set(_default_addr2line ON)
if(WIN32 AND NOT CMAKE_CXX_PLATFORM_ID MATCHES "Cygwin")
  set(_default_addr2line OFF)
endif()

stacktrace_check(BOOST_STACKTRACE_HAS_WINDBG has_windbg.cpp "" "dbgeng;ole32")
stacktrace_check(BOOST_STACKTRACE_HAS_WINDBG_CACHED has_windbg_cached.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../config/include" "dbgeng;ole32")

set(_default_from_exception ON)
if (CMAKE_CXX_PLATFORM_ID MATCHES "Cygwin")
  set(_default_from_exception OFF)
endif()

option(BOOST_STACKTRACE_ENABLE_NOOP "Boost.Stacktrace: build boost_stacktrace_noop" ON)
option(BOOST_STACKTRACE_ENABLE_BACKTRACE "Boost.Stacktrace: build boost_stacktrace_backtrace" ${BOOST_STACKTRACE_HAS_BACKTRACE})
option(BOOST_STACKTRACE_ENABLE_ADDR2LINE "Boost.Stacktrace: build boost_stacktrace_addr2line" ${_default_addr2line})
option(BOOST_STACKTRACE_ENABLE_BASIC "Boost.Stacktrace: build boost_stacktrace_basic" ON)
option(BOOST_STACKTRACE_ENABLE_WINDBG "Boost.Stacktrace: build boost_stacktrace_windbg" ${BOOST_STACKTRACE_HAS_WINDBG})
option(BOOST_STACKTRACE_ENABLE_WINDBG_CACHED "Boost.Stacktrace: build boost_stacktrace_windbg_cached" ${BOOST_STACKTRACE_HAS_WINDBG_CACHED})
option(BOOST_STACKTRACE_ENABLE_FROM_EXCEPTION "Boost.Stacktrace: build boost_stacktrace_from_exception" ${_default_from_exception})

unset(_default_addr2line)
unset(_default_from_exception)

message(STATUS "Boost.Stacktrace: "
  "noop ${BOOST_STACKTRACE_ENABLE_NOOP}, "
  "backtrace ${BOOST_STACKTRACE_ENABLE_BACKTRACE}, "
  "addr2line ${BOOST_STACKTRACE_ENABLE_ADDR2LINE}, "
  "basic ${BOOST_STACKTRACE_ENABLE_BASIC}, "
  "windbg ${BOOST_STACKTRACE_ENABLE_WINDBG}, "
  "windbg_cached ${BOOST_STACKTRACE_ENABLE_WINDBG_CACHED}, "
  "from_exception ${BOOST_STACKTRACE_ENABLE_FROM_EXCEPTION}"
)

add_library(boost_stacktrace_dump)
target_include_directories(boost_stacktrace_dump PRIVATE include)
target_compile_definitions(boost_stacktrace_dump PUBLIC BOOST_USE_MODULES)
target_link_libraries(boost_stacktrace_dump
  PUBLIC
    Boost::config
    Boost::predef
)
target_sources(boost_stacktrace_dump
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/modules
    FILES ${CMAKE_CURRENT_LIST_DIR}/modules/boost_stacktrace_dump.cppm
)
target_sources(boost_stacktrace_dump
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/dump.cpp
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(boost_stacktrace_dump PUBLIC BOOST_STACKTRACE_DYN_LINK)
else()
  target_compile_definitions(boost_stacktrace_dump PUBLIC BOOST_STACKTRACE_STATIC_LINK)
endif()

stacktrace_add_library(noop ${BOOST_STACKTRACE_ENABLE_NOOP} "" "" ${BOOST_USE_MODULES})
stacktrace_add_library(backtrace ${BOOST_STACKTRACE_ENABLE_BACKTRACE} "boost_stacktrace_dump;backtrace;${CMAKE_DL_LIBS}" "" ${BOOST_USE_MODULES})
stacktrace_add_library(addr2line ${BOOST_STACKTRACE_ENABLE_ADDR2LINE} "boost_stacktrace_dump;${CMAKE_DL_LIBS}" "" ${BOOST_USE_MODULES})
stacktrace_add_library(basic ${BOOST_STACKTRACE_ENABLE_BASIC} "boost_stacktrace_dump;${CMAKE_DL_LIBS}" "" ${BOOST_USE_MODULES})
stacktrace_add_library(windbg ${BOOST_STACKTRACE_ENABLE_WINDBG} "dbgeng;ole32" "_GNU_SOURCE=1" ${BOOST_USE_MODULES})
stacktrace_add_library(windbg_cached ${BOOST_STACKTRACE_ENABLE_WINDBG_CACHED} "dbgeng;ole32" "_GNU_SOURCE=1" ${BOOST_USE_MODULES})

# boost_stacktrace, default library

if(BOOST_USE_MODULES)
  add_library(boost_stacktrace)
  target_sources(boost_stacktrace
    PUBLIC
      FILE_SET CXX_MODULES
      BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/modules
      FILES ${CMAKE_CURRENT_LIST_DIR}/modules/boost_stacktrace.cppm
  )
  set(__scope PUBLIC)
else()
  add_library(boost_stacktrace INTERFACE)
  set(__scope INTERFACE)
endif()

target_include_directories(boost_stacktrace ${__scope} include)

add_library(Boost::stacktrace ALIAS boost_stacktrace)

if(BOOST_STACKTRACE_ENABLE_WINDBG)
  set(__default_stacktrace_backend "windbg")
elseif(BOOST_STACKTRACE_ENABLE_WINDBG_CACHED)
  set(__default_stacktrace_backend "windbg_cached")
elseif(BOOST_STACKTRACE_ENABLE_BACKTRACE)
  set(__default_stacktrace_backend "backtrace")
elseif(BOOST_STACKTRACE_ENABLE_ADDR2LINE)
  set(__default_stacktrace_backend "addr2line")
elseif(BOOST_STACKTRACE_ENABLE_BASIC)
  set(__default_stacktrace_backend "basic")
elseif(BOOST_STACKTRACE_ENABLE_NOOP)
  set(__default_stacktrace_backend "noop")
else()
  message(FATAL "All backends are disabled")
endif()

message(STATUS "Boost.stacktrace default backend: ${__default_stacktrace_backend}")
target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_${__default_stacktrace_backend})
unset(__default_stacktrace_backend)

# Boost::stacktrace_from_exception is never the default
stacktrace_add_library(from_exception ${BOOST_STACKTRACE_ENABLE_FROM_EXCEPTION} "boost_stacktrace_dump;${CMAKE_DL_LIBS}" "" FALSE)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
