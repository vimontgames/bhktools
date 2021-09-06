void main()
{
    float2 uv = gl_TexCoord[0].xy;
    float4 height = texture2D(texture, hexUV(uv));

    gl_FragColor.rgba = float4(height.xxx, 1.0f);
}