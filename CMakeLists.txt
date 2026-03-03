# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.8...4.20)

project(boost_stacktrace VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

function(stacktrace_add_library suffix opt libs defs)

  if(NOT opt)
    return()
  endif()

  add_library(boost_stacktrace_${suffix})
  target_sources(boost_stacktrace_${suffix}
    PRIVATE
    src/${suffix}.cpp
  )

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
    PUBLIC BOOST_STACKTRACE_NO_LIB
    PRIVATE BOOST_STACKTRACE_SOURCE ${defs}
  )

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

function(stacktrace_check var source incs libs defs)

  set(CMAKE_REQUIRED_INCLUDES "${incs}")
  list(APPEND CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/build")
  set(CMAKE_REQUIRED_LIBRARIES "${libs}")
  set(CMAKE_REQUIRED_DEFINITIONS "${defs}")
  check_cxx_source_compiles("#include \"${source}\"" ${var})
  set(${var} ${${var}} PARENT_SCOPE)

endfunction()

stacktrace_check(BOOST_STACKTRACE_HAS_BACKTRACE has_backtrace.cpp "" "backtrace" "")

set(_default_addr2line ON)
if(WIN32 AND NOT CMAKE_CXX_PLATFORM_ID MATCHES "Cygwin")
  set(_default_addr2line OFF)
endif()

stacktrace_check(BOOST_STACKTRACE_HAS_WINDBG has_windbg.cpp "" "dbgeng;ole32" "")
stacktrace_check(BOOST_STACKTRACE_HAS_WINDBG_CACHED has_windbg_cached.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../config/include" "dbgeng;ole32" "")

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

# stacktrace_add_library(noop ${BOOST_STACKTRACE_ENABLE_NOOP} "" "")
# stacktrace_add_library(backtrace ${BOOST_STACKTRACE_ENABLE_BACKTRACE} "backtrace;${CMAKE_DL_LIBS}" "")
# stacktrace_add_library(addr2line ${BOOST_STACKTRACE_ENABLE_ADDR2LINE} "${CMAKE_DL_LIBS}" "")
# stacktrace_add_library(basic ${BOOST_STACKTRACE_ENABLE_BASIC} "${CMAKE_DL_LIBS}" "")
# stacktrace_add_library(windbg ${BOOST_STACKTRACE_ENABLE_WINDBG} "dbgeng;ole32" "_GNU_SOURCE=1")
# stacktrace_add_library(windbg_cached ${BOOST_STACKTRACE_ENABLE_WINDBG_CACHED} "dbgeng;ole32" "_GNU_SOURCE=1")

# boost_stacktrace, default library

if(BOOST_USE_MODULES)
  add_library(boost_stacktrace)
  target_sources(boost_stacktrace
    PUBLIC
      FILE_SET modules_public
      TYPE CXX_MODULES
      BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/modules
      FILES ${CMAKE_CURRENT_LIST_DIR}/modules/boost_stacktrace.cppm
    PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/src/backtrace.cpp
  )
  target_link_libraries(boost_stacktrace PRIVATE backtrace ${CMAKE_DL_LIBS})
  
  target_compile_features(boost_stacktrace PUBLIC cxx_std_20)
  target_compile_definitions(boost_stacktrace PUBLIC BOOST_USE_MODULES)
  target_compile_definitions(boost_stacktrace PUBLIC BOOST_STACKTRACE_STATIC_LINK BOOST_STACKTRACE_NO_LIB)
  target_link_libraries(boost_stacktrace
    PUBLIC
      Boost::config
      Boost::container_hash
      Boost::core
      Boost::predef
      Boost::winapi
  )

  message(STATUS "Using modules!")

  set(__scope PUBLIC)
else()
  add_library(boost_stacktrace INTERFACE)
  set(__scope INTERFACE)

  message(STATUS "Not using modules :(: ${__scope}!")
endif()

target_include_directories(boost_stacktrace ${__scope} include)

add_library(Boost::stacktrace ALIAS boost_stacktrace)

if(BOOST_STACKTRACE_ENABLE_WINDBG)

  message(STATUS "Stacktrace backend: WINDBG")
  target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_windbg)

elseif(BOOST_STACKTRACE_ENABLE_WINDBG_CACHED)

  message(STATUS "Stacktrace backend: WINDBG_CACHED")
  target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_windbg)

elseif(BOOST_STACKTRACE_ENABLE_BACKTRACE)

  message(STATUS "Stacktrace backend: BACKTRACE")
  # target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_backtrace)

elseif(BOOST_STACKTRACE_ENABLE_ADDR2LINE)

  message(STATUS "Stacktrace backend: ADDR2LINE")
  # target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_addr2line)

elseif(BOOST_STACKTRACE_ENABLE_BASIC)

  message(STATUS "Stacktrace backend: BASIC")
  target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_basic)

elseif(BOOST_STACKTRACE_ENABLE_NOOP)

  message(STATUS "Stacktrace backend: NOOP")
  target_link_libraries(boost_stacktrace ${__scope} Boost::stacktrace_noop)

endif()

# Boost::stacktrace_from_exception is never the default
stacktrace_add_library(from_exception ${BOOST_STACKTRACE_ENABLE_FROM_EXCEPTION} "${CMAKE_DL_LIBS};boost_stacktrace" "")

#

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
