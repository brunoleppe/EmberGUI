cmake_minimum_required(VERSION 3.14)
project(EmberGUI C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/raylib
)
FetchContent_MakeAvailable(raylib)

add_executable(EmberGUI
    src/main.cpp
    src/render/RectRenderer.cpp
    src/render/Layer.cpp
    src/render/Effect.cpp
    src/render/UI.cpp
)

target_include_directories(EmberGUI PRIVATE src)
target_link_libraries(EmberGUI PRIVATE raylib)