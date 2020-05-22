# Our own configuration set
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release;RelDebug" CACHE STRING "" FORCE)
endif()


# first argument if any - path to root folder
macro(init)
    set (args ${ARGN})

    list(LENGTH args argsLen)
    
    if (${argsLen} GREATER 0)
        list(GET args 0 optArg)
        set (srcRoot ${optArg})
    endif ()

    set (srcRoot ${CMAKE_CURRENT_SOURCE_DIR}/${srcRoot})

    #https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    #if(MSVC)
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    #endif()

    # Can be tested with following code snippet:
    # #define STRX_(x)  #x
    # #define STRX(x)  STRX_(x)
    # #pragma message(STRX(__cplusplus))

    # Typically there is no need to optimize all code parts, but low level stuff like vector, ... makes sense to optimize.
    # It's recommended that developers manually identifies what needs to be optimized and optimizes by himself, 
    # as it eats developers time to build optimized code.
    set(RELDEBUG_OPT_LEVEL "/Od")
    set(MAXIMIZE_SPEED_COMPILE_FLAGS "/O2 /Ob2 /Zo /GS-")
    set(CMAKE_CXX_FLAGS_RELDEBUG "${_MD} /Zi ${RELDEBUG_OPT_LEVEL} /DNDEBUG")
    set(CMAKE_C_FLAGS_RELDEBUG   "${_MD} /Zi ${RELDEBUG_OPT_LEVEL} /DNDEBUG")

    foreach(t EXE SHARED MODULE)
        #https://devblogs.microsoft.com/cppblog/faster-c-build-cycle-in-vs-15-with-debugfastlink/
        #fastlink links 2-5 times faster than debug:full, but some of profilers might want to have /debug:full option enabled instead
        set(CMAKE_${t}_LINKER_FLAGS_RELDEBUG "/debug:fastlink /INCREMENTAL")
    endforeach()
endmacro()

# After include we set srcRoot to be default one
init(..)


macro(boost_project project)
    #/WX - threat warnings as error
    target_compile_options(${project} PRIVATE /W4 /WX)
    target_include_directories(${project} PRIVATE ${srcRoot}/include)
endmacro()

# Gets current platform in short form - "x64" => "64", "Win32" => "32"
macro(get_current_platform)
    set(current_platform ${CMAKE_GENERATOR_PLATFORM})
    if(NOT DEFINED current_platform)
        set(current_platform x64)
    endif()

    if(current_platform STREQUAL "x64")
        set(current_platform 64)
    else()
        if(current_platform STREQUAL "Win32")
            set(current_platform 32)
        endif()
    endif()
endmacro()

