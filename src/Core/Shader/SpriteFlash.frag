#version 400 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;

uniform float alphaMod;

const vec3 COLORS[4] = vec3[](
    vec3(233.0f / 255.0f, 239.0f / 255.0f, 236.0f / 255.0f),
    vec3(160.0f / 255.0f, 160.0f / 255.0f, 139.0f / 255.0f),
    vec3(85.0f / 255.0f, 85.0f / 255.0f, 104.0f / 255.0f),
    vec3(33.0f / 255.0f, 30.0f / 255.0f, 32.0f / 255.0f)
);

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    if (rand(TexCoords.xy) < alphaMod)
        color = vec4(COLORS[0], texture(image, TexCoords).w);
    else
        color = vec4(0);
}
