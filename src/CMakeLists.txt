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
BattleLevel.cpp
Stage1.cpp
LevelBuilder.cpp
PlayableCharacter.cpp
TileMapHelper.cpp
StateMachine.cpp
PhysicsSystem.cpp
PlayerSystem.cpp
RenderSystem.cpp
CameraSystem.cpp
HUDSystem.cpp
InputHandlingSystem.cpp
World.cpp
EnemySystem.cpp
Enemy1.cpp
CommonAI.cpp
AISystem.cpp
NavSystem.cpp
DynamicColliderSystem.cpp
ParticleSystem.cpp
Hit.cpp
BattleSystem.cpp
ChatBox.cpp
ColliderRouting.cpp
EnvironmentSystem.cpp
)

set(INCLUDE_DIRS
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_CURRENT_LIST_DIR}/Core
${SDL2_mixer_INCLUDE_DIRS}
${lz4_INCLUDE_DIRS}
)

message(Paths:)
message(${lz4_INCLUDE_DIRS})

set(LINK_LIBRARIES
${SDL2_LIBRARIES}
SDL2_image::SDL2_image
SDL2_mixer::SDL2_mixer
${lz4_LIBRARIES}
nlohmann_json::nlohmann_json
EnTT::EnTT
)

if (TARGET sdl_ttf::sdl_ttf)
    list(APPEND LINK_LIBRARIES sdl_ttf::sdl_ttf)
else ()
    list(APPEND LINK_LIBRARIES SDL2_ttf::SDL2_ttf)
endif ()

message(${LINK_LIBRARIES})

add_executable (${PROJECT_NAME} ${SRC_FILES})

add_subdirectory (Core)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

include_directories(${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES} Core)