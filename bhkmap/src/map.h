#pragma once

#include "shadermanager.h"
#include "resourceinfo.h"

//--------------------------------------------------------------------------------------
enum MapBitmap
{
    Heightfield = 0,
    Territories,
    Resources,

    Count
};

//--------------------------------------------------------------------------------------
struct Territory
{
    u32 continent = 0;
    u32 biome = 0;
    bool ocean = false;
};

//--------------------------------------------------------------------------------------
struct Bitmap
{
    bool visible = true;

    sf::Sprite sprite;
    sf::Image image;
    sf::Texture texture;

    bool drawQuad = true;
    ShaderID quadshader = invalidShaderID;
    sf::BlendMode quadblend = { sf::BlendMode::Factor::One, sf::BlendMode::Factor::Zero, sf::BlendMode::Add };

    bool drawSprites = false;

    std::vector<sf::Sprite> sprites;
    ShaderID spriteshader = invalidShaderID;
    sf::BlendMode spriteblend = { sf::BlendMode::Factor::One, sf::BlendMode::Factor::Zero, sf::BlendMode::Add };

    float alpha = 1.0f;
};

namespace tinyxml2
{
    class XMLElement;
}

enum class TerritoryBackground : u32
{
    None = 0,
    Territory,
    Biome,

    First = None,
    Last = Biome,
    Count = Last - First
};

//--------------------------------------------------------------------------------------
struct Map
{
    std::string path;
    std::string author;
    u32 width = 0;
    u32 height = 0;

    std::vector<u32> elevation;
    std::vector<u32> zones;
    std::vector<u32> poi;

    std::vector<Territory> territoriesInfo;

    Bitmap bitmaps[MapBitmap::Count];

    bool loaded = false;
    
    TerritoryBackground territoryBackground = TerritoryBackground::Territory;
    bool showTerritoriesBorders = true;

    bool showStrategicResources = false;
    bool showLuxuryResources = false;

    void refresh();

    u32 * loadTexture(tinyxml2::XMLElement * _xmlTerrainSave, const std::string & _name);
    void loadMap(const std::string & _map, const std::string & _cwd);

    sf::Texture strategicResourceTextures[(u32)StrategicResource::Count];
    sf::Texture luxuryResourceTextures[(u32)LuxuryResource::Count];
};