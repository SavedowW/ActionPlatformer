#version 400 core
layout (location = 0) in uint vId;

uniform vec2 vertices[4];
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertices[vId], 0.0, 1.0);
}
