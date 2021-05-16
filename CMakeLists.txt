cmake_minimum_required(VERSION 3.5)
project(skai CXX)

add_executable(main ./main.cpp)

function(set_warns)
    set(msvc_warns /W4 /permissive)
    set(gcc_clang_warns -Wextra -Wall -Wpedantic -Wno-switch)
    if(MSVC)
        set(warns ${msvc_warns})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(warns ${gcc_clang_warns})
    else()
        message(AUTHOR_WARNING "no compiler warnings")
    endif()
    target_compile_options(main PUBLIC ${warns})
endfunction()

target_compile_features(main PUBLIC cxx_std_17)
add_library(libfmt IMPORTED STATIC)
target_link_libraries(main fmt)
target_include_directories(main PUBLIC "${CMAKE_SOURCE_DIR}/include")

set_warns()
