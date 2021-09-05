uniform sampler2D texture;
uniform float2 texSize;
uniform float2 screenSize;

void main()
{
    float2 invScreenSize = 1.0f / screenSize * 1.5f;

    float2 uv = gl_TexCoord[0].xy;

    float4 center   = texture2D(texture, uv);

    float4 left     = texture2D(texture, uv + float2(-invScreenSize.x, 0));
    float4 right    = texture2D(texture, uv + float2(+invScreenSize.x, 0));
    float4 bottom   = texture2D(texture, uv + float2(0, -invScreenSize.y));
    float4 up       = texture2D(texture, uv + float2(0, +invScreenSize.y));

    float4 topLeft     = texture2D(texture, uv + float2(-invScreenSize.x, +invScreenSize.y));
    float4 topRight    = texture2D(texture, uv + float2(+invScreenSize.x, +invScreenSize.y));
    float4 bottomLeft  = texture2D(texture, uv + float2(-invScreenSize.x, -invScreenSize.y));
    float4 bottomRight = texture2D(texture, uv + float2(+invScreenSize.x, -invScreenSize.y));

    bool edge = (0 != (PASS_FLAG_BORDERS & passFlags)) && ((left.a != center.a) || (right.a != center.a) || (bottom.a != center.a) || (up.a != center.a) || (topLeft.a != center.a) || (topRight.a != center.a) || (bottomLeft.a != center.a) || (bottomRight.a != center.a));
   
    float territoryOpacity = 0.5f;
    float edgeOpacity = 0.75f;

    float3 territoryColor[6] =
    {
        float3(1,0,0),
        float3(0,1,0),
        float3(1,1,0),
        float3(1,0,1),
        float3(0,1,1),
        float3(1,1,1)
    };

    float3 biomeColors[10] =
    {
        float3(0.8,0.8,0.9),
        float3(0.6,0.6,0.4),
        float3(1.0,1.0,0.7),
        float3(0.6,0.8,0.3),
        float3(0.8,0.7,0.4),
        float3(1.0,1.0,0.3),
        float3(0.6,1.0,1.0),
        float3(0.5,0.5,0.4),
        float3(0.7,0.8,0.4),
        float3(0.6,0.7,0.6)
    };

    // Desert\0Forest\0Lake\0Mountain\0River
    float3 landmarkColors[5] =
    {
        float3(1,1,0),
        float3(0,1,0),
        float3(0,0,1),
        float3(0.5, 0.25, 0),
        float3(0.5,0.5,1.0)
    };

    uint4 index = uint4(center.rgba * 255.0f);
    
    uint biomeIndex = index.g;
    uint landmarkIndex = index.g;
    uint flags = index.b;
    uint territoryIndex = index.a;
    
    float3 color = float3(0,0,0);
    
    switch (passFlags & 0xF)
    {
        default:
            color = center.g;
            break;

        case PASS_FLAG_TERRITORY:
            color = territoryColor[territoryIndex % 6];
            break;
    
        case PASS_FLAG_LANDMARK:
            color = landmarkColors[landmarkIndex % 5];
            break;
    
        case PASS_FLAG_BIOME:
            color = biomeColors[biomeIndex % 10];
            break;
    }     
       
    float3 edgeColor = float3(1, 1, 1);
    
    bool visible = 0 != (flags & TEXEL_FLAG_VISIBLE);
    bool water = 0 != (flags & TEXEL_FLAG_WATER);
    
    if (water)
    {
        color = lerp(color, float3(0, 0, 1), 0.75f);
        edgeColor = float3(0, 0, 1);
    }

    gl_FragColor.rgba = edge ? float4(edgeColor, edgeOpacity) : float4(color, territoryOpacity);

    if (!visible)
        gl_FragColor.a = 0;
}