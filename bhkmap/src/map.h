#pragma once

#include "types.h"

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
};

namespace tinyxml2
{
    class XMLElement;
}

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
    bool showOceanTerritories = false;
    bool showLandTerritories = true;
    bool showStrategicResources = false;
    bool showLuxuryResources = false;

    void refresh();
    u32 * loadTexture(tinyxml2::XMLElement * _xmlTerrainSave, const std::string & _name);
    void loadMap(const std::string & _map, const std::string & _cwd);
};