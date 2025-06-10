#version 400 core
layout (location = 0) in vec3 idTexCoords;

out vec2 TexCoords;

uniform vec2 vertices[4];
uniform mat4 projection;
uniform float angle;
uniform vec2 pivotPoint;

void main()
{
    TexCoords = idTexCoords.yz;

    vec2 position = vertices[int(idTexCoords.x)] - pivotPoint;

    float cosA = cos(angle);
    float sinA = sin(angle);

    vec2 rotatedPosition;
    rotatedPosition.x = position.x * cosA - position.y * sinA;
    rotatedPosition.y = position.x * sinA + position.y * cosA;

    rotatedPosition += pivotPoint;

    gl_Position = projection * vec4(rotatedPosition, 0.0, 1.0);
}
