cmake_minimum_required(VERSION 3.15)

# Reorganize projects into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Does not have it's own name in Jamfile.v2
add_executable(test_stacktrace test.cpp test_impl.cpp Jamfile.v2)

target_include_directories(test_stacktrace PRIVATE ../../..)
target_compile_definitions(test_stacktrace PRIVATE BOOST_STACKTRACE_USE_WINDBG_CACHED;BOOST_STACKTRACE_DYN_LINK)
set_target_properties(test_stacktrace PROPERTIES FOLDER tests)

target_link_libraries(test_stacktrace PRIVATE boost_stacktrace_windbg)
