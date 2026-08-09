Just a small action platformer I'm working on in my spare time. Uses SDL3, entt, nlohmann-json, openGL, glm, built with cmake. Tiled is used for level editing.

# Animation format
Used to use a custom animation format, now now each directory with json with the same name in animations directory is interpreted as an animation, file postfixes are retrieved from that json.

# Localization

Strings are stored in `Localization/<LANG>/strings.json`. Localization files are not supported yet, though might be necessary (fonts, etc).

# TODOs

[TODOs are here](TODO.md)
