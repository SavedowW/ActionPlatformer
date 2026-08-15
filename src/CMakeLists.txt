cmake_minimum_required (VERSION 3.20)
 
set(PROJECT_NAME ActionPlatformer)

project(${PROJECT_NAME})

set(SRC_FILES 
main.cpp
BattleLevel.cpp
LevelBuilder.cpp
PlayerSystem.cpp
RenderSystem.cpp
CameraSystem.cpp
HUDSystem.cpp
InputHandlingSystem.cpp
EnemySystem.cpp
AISystem.cpp
ParticleSystem.cpp
Hit.cpp
BattleSystem.cpp
ChatBox.cpp
EnvironmentSystem.cpp
EnvComponents.cpp
Physics/DynamicColliderSystem.cpp
Physics/ColliderRouting.cpp
Physics/PhysicsSystem.cpp
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

if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  list(APPEND LINK_LIBRARIES stdc++exp)
endif()

message(Libraries: ${LINK_LIBRARIES})

add_executable (${PROJECT_NAME} ${SRC_FILES})

add_subdirectory (Core)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

include_directories(${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES} Core)
