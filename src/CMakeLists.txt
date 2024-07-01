cmake_minimum_required (VERSION 3.20)
 
set(PROJECT_NAME ActionPlatformer)

project(${PROJECT_NAME})

find_package(SDL2 REQUIRED)
find_package(SDL2_IMAGE REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(lz4 REQUIRED)

set(SRC_FILES 
main.cpp
InputComparators.cpp
BattleLevel.cpp
Stage1.cpp
Background.cpp
FixedQueue.cpp
InputState.cpp
Object.cpp
PlayableCharacter.cpp
)

set(INCLUDE_DIRS
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_CURRENT_LIST_DIR}/Core
${SDL2_mixer_INCLUDE_DIRS}
${lz4_INCLUDE_DIRS}
)

set(LINK_LIBRARIES
${sdl_LIBRARIES_DEBUG}
SDL2::SDL2
SDL2_image::SDL2_image
sdl_ttf::sdl_ttf
${SDL2_mixer_LIBRARIES}
LZ4::lz4_static
)

message(${LINK_LIBRARIES})

add_executable (${PROJECT_NAME} ${SRC_FILES})

add_subdirectory (Core)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

include_directories(${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES} Core)