cmake_minimum_required (VERSION 3.20)
 
project(Core)

set(CORE_SRC_FILES
Timer.cpp
glad.c
InputSystem.cpp
TextureManager.cpp
Texture.cpp
Camera.cpp
Renderer.cpp
Window.cpp
Renderer.cpp
Application.cpp
Level.cpp
AnimationManager.cpp
FrameTimer.cpp
TextManager.cpp
TimelineProperty.cpp
ImmediateScreenLog.cpp
utf8.cpp
InputState.cpp
InputComparators.cpp
InputResolver.cpp
Collider.cpp
CameraFocusArea.cpp
Tileset.cpp
Trigger.cpp
CoreComponents.cpp
NavGraph.cpp
Logger.cpp
Profile.cpp
RectCollider.cpp
FilesystemUtils.cpp
Shader.cpp
)

add_library (Core ${CORE_SRC_FILES})

set_property(TARGET Core PROPERTY CXX_STANDARD 23)

include_directories(${INCLUDE_DIRS})
target_link_libraries(Core ${LINK_LIBRARIES})