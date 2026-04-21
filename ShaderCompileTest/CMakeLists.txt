cmake_minimum_required(VERSION 3.16)
project(ShaderCompileTest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find Vulkan SDK ──────────────────────────────────────────────────
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found: ${Vulkan_LIBRARIES}")

# ── OpenGL ───────────────────────────────────────────────────────────
# We do NOT use find_package(OpenGL) because its imported target adds
# the system GL/gl.h include path, which conflicts with our GLAD loader.
# On Windows we link opengl32.lib directly.
if(WIN32)
    set(OPENGL_LIB opengl32)
else()
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIB OpenGL::GL)
endif()

# ── GLFW (window + context creation) ────────────────────────────────
# We use FetchContent so the user doesn't need to install GLFW manually.
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ── GLAD (OpenGL loader) ────────────────────────────────────────────
# We ship a minimal glad loader in third_party/glad/
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# ── Benchmark executable ────────────────────────────────────────────
add_executable(shader_compile_test
    src/main.cpp
    src/vulkan_test.cpp
    src/opengl_test.cpp
    src/utils.cpp
)

target_include_directories(shader_compile_test PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    src/
)

target_link_libraries(shader_compile_test PRIVATE
    Vulkan::Vulkan
    ${OPENGL_LIB}
    glfw
    glad
)

# ── Copy shaders to build directory ─────────────────────────────────
add_custom_command(TARGET shader_compile_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:shader_compile_test>/shaders
    COMMENT "Copying shader files to build directory"
)

# ── Compile SPIR-V shaders (requires glslangValidator in PATH) ──────
find_program(GLSLANG_VALIDATOR glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin")
if(GLSLANG_VALIDATOR)
    message(STATUS "glslangValidator found: ${GLSLANG_VALIDATOR}")

    set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
    set(SPIRV_DIR  ${CMAKE_SOURCE_DIR}/shaders/spirv)
    file(MAKE_DIRECTORY ${SPIRV_DIR})

    set(GLSL_SOURCES
        ${SHADER_DIR}/heavy.vert
        ${SHADER_DIR}/heavy.frag
        ${SHADER_DIR}/baseline.vert
        ${SHADER_DIR}/baseline.frag
    )

    foreach(GLSL_FILE ${GLSL_SOURCES})
        get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
        set(SPIRV_FILE ${SPIRV_DIR}/${FILE_NAME}.spv)
        add_custom_command(
            OUTPUT  ${SPIRV_FILE}
            COMMAND ${GLSLANG_VALIDATOR} -V ${GLSL_FILE} -o ${SPIRV_FILE}
            DEPENDS ${GLSL_FILE}
            COMMENT "Compiling ${FILE_NAME} -> SPIR-V"
        )
        list(APPEND SPIRV_BINARIES ${SPIRV_FILE})
    endforeach()

    add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_BINARIES})
    add_dependencies(shader_compile_test compile_shaders)
else()
    message(WARNING "glslangValidator NOT found – SPIR-V shaders will NOT be compiled automatically.\n"
                    "Please compile shaders/heavy.vert and shaders/heavy.frag manually.")
endif()
