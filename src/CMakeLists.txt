cmake_minimum_required (VERSION 3.20)
 
set(PROJECT_NAME ActionPlatformer)

project(${PROJECT_NAME})

set(SRC_FILES 
main.cpp
BattleLevel.cpp
LevelBuilder.cpp
PlayableCharacter.cpp
StateMachine.cpp
StateMachineEx.cpp
StateMachineFactory.cpp
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
DynamicColliderSystem.cpp
ParticleSystem.cpp
Hit.cpp
BattleSystem.cpp
ChatBox.cpp
ColliderRouting.cpp
EnvironmentSystem.cpp
EnvComponents.cpp
ResetHandlers.cpp
)

set(INCLUDE_DIRS
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_SOURCE_DIR}/external/glm
)

message(Paths:)

set(LINK_LIBRARIES
SDL3::SDL3
SDL3_image::SDL3_image
SDL3_ttf::SDL3_ttf
SDL3_mixer::SDL3_mixer
nlohmann_json::nlohmann_json
EnTT::EnTT
)

message(${LINK_LIBRARIES})

add_executable (${PROJECT_NAME} ${SRC_FILES})

add_subdirectory (Core)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

include_directories(${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES} Core)
