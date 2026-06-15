cmake_minimum_required(VERSION 3.16)

project(blit)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

add_library(blit STATIC
    src/window.cpp
)

add_executable(demo
    examples/demo.cpp
)

target_include_directories(blit
    PUBLIC
    include
)

target_link_libraries(blit
    PUBLIC
    SDL2::SDL2
)

target_link_libraries(demo
    PRIVATE blit
)

add_executable(demo2
    examples/demo2.cpp
)

target_link_libraries(demo2
    PRIVATE blit
)