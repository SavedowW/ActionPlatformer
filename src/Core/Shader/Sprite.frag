#version 400 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform float alphaMod;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec4 originalColor = texture(image, TexCoords);

    if (rand(TexCoords.xy) < alphaMod)
        color = texture(image, TexCoords) * vec4(1, 1, 1, originalColor.w);
    else
        color = vec4(0);
}
