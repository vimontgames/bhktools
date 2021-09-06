#ifdef __cplusplus
#pragma once
#else
uniform int passFlags;
#endif

#define PASS_FLAG_TERRITORY         0x00000001
#define PASS_FLAG_LANDMARK          0x00000002
#define PASS_FLAG_BIOME             0x00000003
#define PASS_FLAG_WONDER            0x00000004

#define PASS_FLAG_BORDERS           0x80000000

#define TEXEL_FLAG_WATER            0x40
#define TEXEL_FLAG_VISIBLE          0x80

