#pragma once

//--------------------------------------------------------------------------------------
inline float clamp(float _val, float _min, float _max)
{
    return _val < 0.0f ? 0.0f : (_val > 1.0f ? 1.0f : _val);
}

//--------------------------------------------------------------------------------------
inline float saturate(float _val)
{
    return clamp(_val, 0.0f, 1.0f);
}