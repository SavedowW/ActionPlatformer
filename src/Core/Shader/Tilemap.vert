#version 400 core
layout (location = 0) in vec3 idTexCoords;

out vec2 TexCoords;

uniform sampler2D image;
uniform vec2 vertices[4];
uniform mat4 projection;
uniform vec2 tilePos; // In pixels

const vec2 TILE_SIZE = vec2(16, 16); // In pixels

void main()
{
    vec2 texSize = textureSize(image, 0); // pixels

    vec2 normTileTL = vec2(tilePos.x / texSize.x, tilePos.y / texSize.y);
    vec2 normTileSize = vec2(TILE_SIZE.x / texSize.x, TILE_SIZE.y / texSize.y);

    TexCoords.x = normTileTL.x + idTexCoords.y * normTileSize.x;
    TexCoords.y = normTileTL.y + idTexCoords.z * normTileSize.y;

    gl_Position = projection * vec4(vertices[int(idTexCoords.x)], 0.0, 1.0);
}
