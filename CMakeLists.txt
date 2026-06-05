cmake_minimum_required(VERSION 3.14)
project(EmberGUI C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch Sokol into the external/sokol directory
FetchContent_Declare(
    sokol
    GIT_REPOSITORY https://github.com/floooh/sokol.git
    GIT_TAG        master
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/sokol
)
FetchContent_MakeAvailable(sokol)

add_executable(EmberGUI src/main.cpp)

# Make Sokol headers available to the project
target_include_directories(EmberGUI PRIVATE ${sokol_SOURCE_DIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Link mandatory Linux system dependencies
    target_link_libraries(EmberGUI PRIVATE
            X11
            Xi
            Xcursor
            GLESv2
            EGL
            pthread
            m
    )
endif()