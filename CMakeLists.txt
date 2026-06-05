cmake_minimum_required(VERSION 3.14)
project(EmberGUI C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    sokol
    GIT_REPOSITORY https://github.com/floooh/sokol.git
    GIT_TAG        master
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/sokol
)
FetchContent_MakeAvailable(sokol)

# Download sokol-tools-bin (pre-built binaries — no CMakeLists to configure).
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
    sokol_tools
    GIT_REPOSITORY https://github.com/floooh/sokol-tools-bin.git
    GIT_TAG        master
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/sokol-tools-bin
)
FetchContent_MakeAvailable(sokol_tools)

if(WIN32)
    set(SOKOL_SHDC "${sokol_tools_SOURCE_DIR}/bin/win32/sokol-shdc.exe")
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(SOKOL_SHDC "${sokol_tools_SOURCE_DIR}/bin/osx_arm64/sokol-shdc")
    else()
        set(SOKOL_SHDC "${sokol_tools_SOURCE_DIR}/bin/osx/sokol-shdc")
    endif()
else()
    set(SOKOL_SHDC "${sokol_tools_SOURCE_DIR}/bin/linux/sokol-shdc")
endif()

include(cmake/SokolShaders.cmake)

add_executable(EmberGUI src/main.cpp)

target_include_directories(EmberGUI PRIVATE ${sokol_SOURCE_DIR})

if(WIN32)
    target_compile_definitions(EmberGUI PRIVATE SOKOL_D3D11)
    target_link_libraries(EmberGUI PRIVATE d3d11 dxgi)
    set_target_properties(EmberGUI PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(EmberGUI PRIVATE SOKOL_GLES3)
    target_link_libraries(EmberGUI PRIVATE X11 Xi Xcursor GLESv2 EGL pthread m)
endif()

target_add_sokol_shader(EmberGUI Shaders/triangle.glsl)
