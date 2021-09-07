#pragma once

#include "shadermanager.h"
#include "resourceinfo.h"
#include "tinyxml2.h"

#define MAX_PLAYER_SPAWN 8

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
    Tile,
    Territory,
    Biome,
    Landmarks,
    Wonders,

    First = None,
    Last = Wonders,
    Count = Last - First + 1
};

//--------------------------------------------------------------------------------------
struct NaturalWonder
{
    std::string name;
    ColorFloat4 color = ColorFloat4(1,0,0,1);
    bool visible = true;
};

//--------------------------------------------------------------------------------------
struct SpawnPoint
{
    sf::Vector2u pos;
    u32 flags = 0;
};

//--------------------------------------------------------------------------------------
struct SpawnInfo
{
    bool visible = false;
    sf::Texture texture;
    std::vector<SpawnPoint> spawns;
};

//--------------------------------------------------------------------------------------
struct Map
{
    void clearTerritories();
    void clearLandmarks();

    void loadIcons();
    void refresh();
    
    u32 * loadTexture(tinyxml2::XMLElement * _xmlTerrainSave, const std::string & _name);

    bool loadHMap(const std::string & _map, const std::string & _cwd);
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
    //std::vector<NaturalWonder> naturalWondersInfo;

    Bitmap bitmaps[MapBitmap::Count];

    bool loaded = false;
    
    TerritoryBackground territoryBackground = TerritoryBackground::Territory;
    bool showTerritoriesBorders = true;
    bool useHexUVs = true;

    bool showStrategicResources = false;
    bool showLuxuryResources = false;
    bool showWonders = false;
    bool showSpawnPoints = false;

    tinyxml2::XMLDocument xmlDocDescriptor;
    tinyxml2::XMLDocument xmlDocSave;

    bool overridePlayerSpawns = true;
    bool overrideLandmarks = true;

    SpawnInfo spawnInfo[MAX_PLAYER_SPAWN];
    u32 spawnPlayerCountDisplayed = 0;
};