#pragma once

//--------------------------------------------------------------------------------------
struct ColorFloat4
{
    inline ColorFloat4(float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 1.0f) :
        r(_r), g(_g), b(_b), a(_a)
    {

    }

    union
    {
        struct { float r, g, b, a; };
        float rgba[4];
    };
};