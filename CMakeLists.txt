cmake_minimum_required(VERSION 3.20)
project(game VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include Sanitizers
include(cmake/Sanitizers.cmake)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/core/box.cpp
    src/core/collision_detection.cpp
    src/core/enemy.cpp
    src/core/enemy_manager.cpp
    src/core/glfw_setup.cpp
    src/core/player.cpp
    src/core/ray.cpp
    src/core/world.cpp
    src/render/glad.c
    src/render/hud.cpp
    src/render/mesh.cpp
    src/render/model.cpp
    src/render/shader.cpp
    src/render/skybox.cpp
    src/render/text_renderer.cpp
    src/render/texture_loading.cpp
    src/render/stb_image.cpp
    src/audio/miniaudio.c
)

add_executable(game ${SOURCE_FILES})

# Include directories
target_include_directories(game PRIVATE 
    src/core
    src/render
    src/audio
    include
)

# Asset Path Definition
target_compile_definitions(game PRIVATE
    ASSET_PATH="${CMAKE_SOURCE_DIR}/assets"
)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(ASSIMP REQUIRED assimp)
pkg_check_modules(FREETYPE REQUIRED freetype2)

target_include_directories(game PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

target_link_directories(game PRIVATE
    ${GLFW_LIBRARY_DIRS}
    ${ASSIMP_LIBRARY_DIRS}
    ${FREETYPE_LIBRARY_DIRS}
)

if (APPLE)
    target_compile_definitions(game PRIVATE MA_NO_RUNTIME_LINKING)
    target_link_libraries(game PRIVATE
        ${GLFW_LIBRARIES}
        ${ASSIMP_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
    )
elseif (UNIX)
    target_link_libraries(game PRIVATE
        ${GLFW_LIBRARIES}
        ${ASSIMP_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        GL
        X11
        pthread
        dl
    )
endif()

# Output directory
set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Testing
enable_testing()
add_executable(test_math tests/test_math.cpp)
add_test(NAME MathTest COMMAND test_math)
