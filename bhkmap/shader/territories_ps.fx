uniform sampler2D texture;
uniform float2 texSize;
uniform float2 screenSize;
uniform uint flags;
uniform bool borders;

#define TERRITORIES_FLAGS_WATER   0x40
#define TERRITORIES_FLAGS_VISIBLE 0x80

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

    bool edge = borders && ((left.a != center.a) || (right.a != center.a) || (bottom.a != center.a) || (up.a != center.a) || (topLeft.a != center.a) || (topRight.a != center.a) || (bottomLeft.a != center.a) || (bottomRight.a != center.a));
   
    float territoryOpacity = 0.5f;
    float edgeOpacity = 0.75f;

    float3 colors[6] =
    {
        float3(1,0,0),
        float3(0,1,0),
        float3(1,1,0),
        float3(1,0,1),
        float3(0,1,1),
        float3(1,1,1)
    };

    int index = int(center.a * 255.0f);

    float3 color = colors[index % 6];
    float3 edgeColor = float3(1, 1, 1);

    uint flags = int(center.b * 255.0f);
    bool visible = 0 != (flags & TERRITORIES_FLAGS_VISIBLE);
    bool water = 0 != (flags & TERRITORIES_FLAGS_WATER);

    if (water)
    {
        color = float3(0, 0, 1);
        edgeColor = float3(0, 0, 1);
    }

    gl_FragColor.rgba = edge ? float4(edgeColor, edgeOpacity) : float4(color, territoryOpacity);

    if (!visible)
        gl_FragColor.a = 0;
}