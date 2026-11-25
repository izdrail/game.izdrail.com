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
    src/world/terrain.cpp
    src/data/json_mini.cpp
    src/data/weapons.cpp
    src/data/weapon_state.cpp
    src/world/roads.cpp
    src/world/outpost_layout.cpp
    src/render/glad.c
    src/render/hud.cpp
    src/render/mesh.cpp
    src/render/model.cpp
    src/render/shader.cpp
    src/render/skybox.cpp
    src/render/terrain_renderer.cpp
    src/render/outpost_renderer.cpp
    src/render/weapon_model.cpp
    src/render/prop_library.cpp
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
    src/world
    src/data
    include
)

# Asset Path Definition. Packaged builds pass -DGAME_ASSET_PATH=assets and are
# started through run.sh / run.command, which cd into the package directory.
set(GAME_ASSET_PATH "${CMAKE_SOURCE_DIR}/assets" CACHE STRING "Asset directory baked into the binary")
target_compile_definitions(game PRIVATE
    ASSET_PATH="${GAME_ASSET_PATH}"
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
add_executable(test_terrain tests/test_terrain.cpp src/world/terrain.cpp src/world/roads.cpp)
add_test(NAME TerrainTest COMMAND test_terrain)
add_executable(test_roads tests/test_roads.cpp src/world/terrain.cpp src/world/roads.cpp src/world/outpost_layout.cpp)
add_test(NAME RoadsTest COMMAND test_roads)

add_executable(test_weapons tests/test_weapons.cpp src/data/json_mini.cpp src/data/weapons.cpp)
target_compile_definitions(test_weapons PRIVATE WEAPONS_JSON="${CMAKE_SOURCE_DIR}/assets/data/weapons.json")
add_test(NAME WeaponsTest COMMAND test_weapons)

add_executable(test_weapon_state tests/test_weapon_state.cpp src/data/json_mini.cpp src/data/weapons.cpp src/data/weapon_state.cpp)
target_compile_definitions(test_weapon_state PRIVATE WEAPONS_JSON="${CMAKE_SOURCE_DIR}/assets/data/weapons.json")
add_test(NAME WeaponStateTest COMMAND test_weapon_state)
