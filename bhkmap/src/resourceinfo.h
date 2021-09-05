#pragma once

#include "colorfloat4.h"

//--------------------------------------------------------------------------------------
struct ResourceInfo
{
    ColorFloat4 color;
    std::string name;
    bool visible;
};

//--------------------------------------------------------------------------------------
// https://humankind.fandom.com/wiki/Strategic_Resource
//--------------------------------------------------------------------------------------
enum class StrategicResource
{
    Horse = 0x19,
    Copper,
    Iron,
    Coal,
    Salpetre,
    Oil,
    Aluminium,
    Uranium,

    First = Horse,
    Last = Uranium,
    Count = Last-First
};

//--------------------------------------------------------------------------------------
// https://humankind.fandom.com/wiki/Luxury_Resource
//--------------------------------------------------------------------------------------
enum class LuxuryResource
{
    Salt = 0x21,
    Sage,
    Coffee,
    Tea,
    Saffron,

    Dye,
    Ebony,
    Marble,
    Obsidian,
    Silk,

    Incense,
    Porcelain,
    Pearls,
    Gold,
    Gemstone,

    Ambergris,
    Papyrus,
    Lead,
    Mercury,
    Silver,

    Weapon,

    First = Salt,
    Last = Weapon,
    Count = Last - First
};

extern ResourceInfo luxuryResources[];
extern ResourceInfo strategicResources[];