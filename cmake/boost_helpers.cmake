
# first argument if any - path to root folder
macro(init)
    set (args ${ARGN})

    list(LENGTH args argsLen)
    
    if (${argsLen} GREATER 0)
        list(GET args 0 optArg)
        set (srcRoot ${optArg})
    endif ()

    set (srcRoot ${CMAKE_CURRENT_SOURCE_DIR}/${srcRoot})
endmacro()

# After include we set srcRoot to be default one
init(..)


macro(boost_project project)
    #target_compile_options(${project} PRIVATE /W4 /WX)
    target_include_directories(${project} PRIVATE ${srcRoot}/include)
endmacro()

