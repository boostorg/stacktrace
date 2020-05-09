
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

endmacro()

# After include we set srcRoot to be default one
init(..)


macro(boost_project project)
    #target_compile_options(${project} PRIVATE /W4 /WX)
    target_compile_options(${project} PRIVATE /W4)
    target_include_directories(${project} PRIVATE ${srcRoot}/include)
endmacro()

