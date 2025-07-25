Just a small action platformer I'm working on in my spare time. Uses SDL2, lz4 (these 2 are resolved via conan), entt, nlohmann-json (git submodules), built with cmake. Tiled is used for tilemap editing, the game only needs JSON map and images of tilesets
# About animation format
~~Detalis are in SavedowW/AnimationEditor, but the way it is right now in this project:~~
- ~~There are 3 (hardcoded) layers of LZ4-compressed sprites: RGB, pure white with (optional) glow and white border (not necessary for this project in particular)~~
- ~~All individual images have the same resolution, but might have different size after compression~~
- ~~Animation speed is tied to the framerate, non-unique frames (images) are shared between respective frames (in time)~~

Now each directory with json with the same name in animations directory is interpreted as an animation, file postfixes are retrieved from that json.
# Building on windows
Assuming that you have CMake and conan installed and configured and you are in the project folder, run this:
```
conan install . --output-folder=build --build=missing --profile=<debug / release / any other profile you have> (can also use one of *.bat files in the folder if you are using windows)
cd ./build
cmake .. -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
cmake --buid .
```
If you are not using conan, make sure to provide dependencies to CMake manually
# TODOs
### Short-term goals / necessary for release:
- [DONE, but physics bs never ends] Use ints for position but keep floats for velocity / inertia - celeste uses similar system and it works fine. Currently, floats cause a lot of headache due to inaccuracy (0.1 + 0.2 - 0.3) and edgecases, some of which cause visible jiggle for moving objects
- Normalize camera behavior while following moving player, especially on the moving platforms and while wall jumping
- More content, mechanics (planned or not) in general (obviously)
- Level enter - leave behavior, transitions
- Menus
- [Serialization DONE] Serialize input system, make inputs configurable
- [DONE] Add gamepad support (is it really necessary?)
- [DONE] Make so that tile layers are automatically handled the last
- Simplify World component, it shouldn't provide access to camera or navigation system
- BETTER Configs for debug stuff and settings
- Rework AI component, split logic for enemies
### Optional stuff
- Fade-out with resize or dithering instead of alpha - not necessary, might be worse, but worth trying
- Gamepad rumble - on hard landing, hit, whenever the screen is shaking, etc
### Optimizations
- Multithreading - should be at least possible with ECS
- [DONE] Render only tiles within camera scope - shouldn't be too hard and probably will become necessary anyway
- Collect colliders into chunks to optimize collision detection - it depends
### Dreams
- Automated building of pathfinding graph
- Reorganize components and systems for multithreaded optimization (the way ECS is supposed to be used)
- [DONE] Fade out with dithering mask instead of alpha channel
- 2D lighting, preferably with field of vision or something similar (https://www.redblobgames.com/articles/visibility/) and nes / gb - style pallete gradation instead of just plain brightenss - most likely not in this game, but who knows
- [DONE] OpenGL transition - most likely not in this game
