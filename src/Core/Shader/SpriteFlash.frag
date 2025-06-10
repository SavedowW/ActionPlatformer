#version 400 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;

uniform vec4 flashColor;

void main()
{
    color =  vec4(flashColor.xyz, texture(image, TexCoords).w * flashColor.w);
}
