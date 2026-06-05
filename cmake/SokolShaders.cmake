# Requires SOKOL_SHDC to be set (path to the sokol-shdc executable).
# Only compiles the slang needed for the current platform.

if(WIN32)
    set(_SOKOL_SLANG "hlsl5")
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(_SOKOL_SLANG "metal_macos")
    else()
        set(_SOKOL_SLANG "metal_macos")
    endif()
else()
    set(_SOKOL_SLANG "glsl300es")
endif()

# target_add_sokol_shader(<target> <glsl_file>)
#   Compiles <glsl_file> with sokol-shdc and makes the generated header
#   available to <target> without any extra include_directories() calls.
function(target_add_sokol_shader TARGET SHADER_FILE)
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
    get_filename_component(SHADER_ABS  "${SHADER_FILE}" ABSOLUTE)

    set(OUT_DIR  "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    set(OUT_FILE "${OUT_DIR}/${SHADER_NAME}.h")

    add_custom_command(
        OUTPUT  "${OUT_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUT_DIR}"
        COMMAND "${SOKOL_SHDC}"
                --input  "${SHADER_ABS}"
                --output "${OUT_FILE}"
                --slang  "${_SOKOL_SLANG}"
                --format sokol
        DEPENDS "${SHADER_ABS}"
        COMMENT "Compiling shader: ${SHADER_NAME}.glsl"
        VERBATIM
    )

    target_sources(${TARGET} PRIVATE "${OUT_FILE}")
    target_include_directories(${TARGET} PRIVATE "${OUT_DIR}")
endfunction()
