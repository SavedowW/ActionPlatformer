set(JSON_BuildTests OFF CACHE INTERNAL "")
set(SDLTTF_VENDORED ON)

add_subdirectory(json)
add_subdirectory(entt)
add_subdirectory(glm)

add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)
