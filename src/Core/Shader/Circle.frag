#version 400 core
out vec4 pxCol;

uniform vec4 color;
uniform float radius;
uniform vec2 center;

void main()
{
    vec2 pxPos = vec2(gl_FragCoord.x, 360 - gl_FragCoord.y);
    float pxRad = length(pxPos - center);
    float diff = radius - pxRad;
    if (diff <= 1.0 && diff >= 0.0)
        pxCol = color;
    else
        discard;
}
