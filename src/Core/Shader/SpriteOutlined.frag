#version 400 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D backi;
uniform sampler2D image;

const vec4 COLOR_LVL1 = vec4(233.0f / 255.0f, 239.0f / 255.0f, 236.0f / 255.0f, 1.0f);
const vec4 COLOR_LVL2 = vec4(160.0f / 255.0f, 160.0f / 255.0f, 139.0f / 255.0f, 1.0f);
const vec4 COLOR_LVL3 = vec4(85.0f / 255.0f, 85.0f / 255.0f, 104.0f / 255.0f, 1.0f);
const vec4 COLOR_LVL4 = vec4(33.0f / 255.0f, 30.0f / 255.0f, 32.0f / 255.0f, 1.0f);

bool isCloseEnough(vec3 col_)
{
    vec3 colDiff = col_ - COLOR_LVL4.xyz;
    return (abs(colDiff.x) + abs(colDiff.y) + abs(colDiff.z)) < 0.01;
}

void main()
{
    vec4 px = texture(image, TexCoords);
    vec2 backPos = vec2(gl_FragCoord.x / 640.0f, gl_FragCoord.y / 360.0f);
    if (px.w > 0)
    {
        color = px;
    }
    else if (isCloseEnough(texture(backi, backPos).xyz))
    {
        /*const ivec2 offsets[4] = const ivec2[](
            ivec2(0, 1),
            ivec2(0, -1),
            ivec2(1, 0),
            ivec2(-1, 0)
        );

        vec4 res = textureGatherOffsets(image, TexCoords, offsets, 3);

        float sum = res.x + res.y + res.z + res.w;*/

        float sum = (isCloseEnough(textureOffset(backi, backPos, ivec2(0, -1)).xyz) ? textureOffset(image, TexCoords, ivec2(0, 1)).w : 0)
                  + (isCloseEnough(textureOffset(backi, backPos, ivec2(0, 1)).xyz) ? textureOffset(image, TexCoords, ivec2(0, -1)).w : 0)
                  + (isCloseEnough(textureOffset(backi, backPos, ivec2(1, 0)).xyz) ? textureOffset(image, TexCoords, ivec2(1, 0)).w : 0)
                  + (isCloseEnough(textureOffset(backi, backPos, ivec2(-1, 0)).xyz) ? textureOffset(image, TexCoords, ivec2(-1, 0)).w : 0);
        
        if (sum > 0)
            color = COLOR_LVL3;
        else
            color = vec4(0, 0, 0, 0);
    }
    else
    {
        color = vec4(0, 0, 0, 0);
    }
}
