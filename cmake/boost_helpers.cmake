
macro(boost_project project)
    target_compile_options(${project} PRIVATE /W4 /WX)
endmacro()

