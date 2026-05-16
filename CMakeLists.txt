cmake_minimum_required(VERSION 3.16)
project(TheLastSignal VERSION 1.0 LANGUAGES CXX)

# Modern C++ standard, enforced.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(novel
    src/main.cpp
    src/Game.cpp
    src/States.cpp
    scenes/Story.cpp
)

target_include_directories(novel PRIVATE include)

if(MSVC)
    target_compile_options(novel PRIVATE /W4)
else()
    target_compile_options(novel PRIVATE -Wall -Wextra -Wpedantic)
endif()
