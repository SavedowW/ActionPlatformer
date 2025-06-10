#version 400 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform float alphaMod;

void main()
{
    color = texture(image, TexCoords) * vec4(1, 1, 1, alphaMod);
}
