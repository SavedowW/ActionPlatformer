#version 400 core
layout (location = 0) in vec3 idTexCoords;

out vec2 TexCoords;

uniform vec2 vertices[4];
uniform mat4 projection;

void main()
{
    TexCoords = idTexCoords.yz;
    gl_Position = projection * vec4(vertices[int(idTexCoords.x)], 0.0, 1.0);
}
