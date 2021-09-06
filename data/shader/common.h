#ifdef __cplusplus
#pragma once
#endif

#define PASS_FLAG_TERRITORY 0x00000001
#define PASS_FLAG_LANDMARK  0x00000002
#define PASS_FLAG_BIOME     0x00000003
#define PASS_FLAG_WONDER    0x00000004

#define PASS_FLAG_BORDERS   0x40000000
#define PASS_FLAG_HEXES     0x80000000

#define TEXEL_FLAG_WATER    0x40
#define TEXEL_FLAG_VISIBLE  0x80

#ifndef __cplusplus
uniform int passFlags;
uniform float2 texSize;
uniform float2 screenSize;
uniform float4 color;
uniform sampler2D texture;

float2 hexUV(float2 uv)
{
    if (PASS_FLAG_HEXES & passFlags)
    {
        if (int(uv.y * texSize.y) & 1)
            uv.x += 0.25f / texSize.x;
        else
            uv.x -= 0.25f / texSize.x;
    }
    return uv;
}
#endif

