#pragma once

#include "shadermanager.h"
#include "resourceinfo.h"
#include "tinyxml2.h"

//--------------------------------------------------------------------------------------
enum MapBitmap
{
    Heightfield = 0,
    Territories,
    Resources,

    First = Heightfield,
    Last = Resources,
    Count = Last - First + 1
};

//--------------------------------------------------------------------------------------
struct Territory
{
    u32 continent = 0;
    u32 biome = 0;
    bool ocean = true;
};

//--------------------------------------------------------------------------------------
struct Landmark
{
    std::string name;
    u32 definitonIndex = 0;
    std::vector<sf::Vector2u> positions;
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

//--------------------------------------------------------------------------------------
enum class TerritoryBackground : u32
{
    None = 0,
    Territory,
    Biome,
    Landmarks,
    NaturalWonders,

    First = None,
    Last = NaturalWonders,
    Count = Last - First + 1
};

//--------------------------------------------------------------------------------------
struct Map
{
    void clearTerritories();
    void clearLandmarks();

    void refresh();

    u32 * loadTexture(tinyxml2::XMLElement * _xmlTerrainSave, const std::string & _name);

    void loadHMap(const std::string & _map, const std::string & _cwd);
    void saveHMap(std::string & _map, const std::string & _cwd);

    std::string path;
    std::string author;

    u32 width = 0;
    u32 height = 0;

    std::vector<u32> elevation;
    std::vector<u32> zones;
    std::vector<u32> poi;
    std::vector<u32> landmarks;
    std::vector<u32> naturalwonders;

    std::vector<Territory> territoriesInfo;
    std::vector<Landmark> landmarkInfo;
    std::vector<std::string> naturalWonderNames;

    Bitmap bitmaps[MapBitmap::Count];

    bool loaded = false;
    
    TerritoryBackground territoryBackground = TerritoryBackground::Territory;
    bool showTerritoriesBorders = true;

    bool showStrategicResources = false;
    bool showLuxuryResources = false;

    sf::Texture strategicResourceTextures[(u32)StrategicResource::Count];
    sf::Texture luxuryResourceTextures[(u32)LuxuryResource::Count];

    tinyxml2::XMLDocument xmlDocDescriptor;
    tinyxml2::XMLDocument xmlDocSave;

    bool exportTerritories = false;
    bool exportLandmarks = false;
};