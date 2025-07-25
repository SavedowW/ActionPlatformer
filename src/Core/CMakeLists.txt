cmake_minimum_required (VERSION 3.20)
 
project(Core)

file(GLOB_RECURSE files ${CMAKE_SOURCE_DIR}/Localization/*/strings.json)
message(files: ${files})

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_LIST_DIR}/Localization/LocalizationGen.h
        ${CMAKE_CURRENT_LIST_DIR}/Localization/LocalizationGen.cpp
    COMMAND python ${CMAKE_CURRENT_LIST_DIR}/Localization/genLocalization.py
        --cpp ${CMAKE_CURRENT_LIST_DIR}/Localization/LocalizationGen.cpp
        --header ${CMAKE_CURRENT_LIST_DIR}/Localization/LocalizationGen.h
        --jsonroot ${CMAKE_SOURCE_DIR}/Localization
    COMMENT "Generating localization files"
    DEPENDS ${files} ${CMAKE_CURRENT_LIST_DIR}/Localization/genLocalization.py
)

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
Configuration.cpp
Localization/LocalizationGen.cpp
)

add_library (Core ${CORE_SRC_FILES})

set_property(TARGET Core PROPERTY CXX_STANDARD 23)

include_directories(${INCLUDE_DIRS})
target_link_libraries(Core ${LINK_LIBRARIES})