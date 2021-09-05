#include "map.h"
#include "resourceinfo.h"
#include "maths.h"
#include "tinyxml2.h"
#include "zip_file.hpp"
#include "base64.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define TERRITORIES_FLAGS_BIOME   0x20
#define TERRITORIES_FLAGS_WATER   0x40
#define TERRITORIES_FLAGS_VISIBLE 0x80

using namespace std;
using namespace sf;
using namespace tinyxml2;

//--------------------------------------------------------------------------------------
Color ColorFloat4ToUbyte4(const ColorFloat4 & _color)
{
    return Color
    (
        ubyte(saturate(_color.r) * 255.0f + 0.5f),
        ubyte(saturate(_color.g) * 255.0f + 0.5f),
        ubyte(saturate(_color.b) * 255.0f + 0.5f),
        ubyte(saturate(_color.a) * 255.0f + 0.5f)
    );
}

//--------------------------------------------------------------------------------------
void Map::refresh()
{
    if (!loaded)
        return;

    // load resource textures if needed
    for (u32 i = 0; i < (u32)StrategicResource::Count; ++i)
    {
        Texture & tex = strategicResourceTextures[i];
        if (Vector2u(0, 0) == tex.getSize())
            tex.loadFromFile("data/img/" + strategicResources[i].name + ".png");
    }

    for (u32 i = 0; i < (u32)LuxuryResource::Count; ++i)
    {
        Texture & tex = luxuryResourceTextures[i];
        if (Vector2u(0, 0) == tex.getSize())
            tex.loadFromFile("data/img/" + luxuryResources[i].name + ".png");
    }

    for (u32 i = 0; i < MapBitmap::Count; ++i)
    {
        auto & bitmap = bitmaps[i];
        bitmap.image.create(width, height);
        bitmap.sprites.clear();
    }

    Bitmap & heigthfield = bitmaps[Heightfield];
    Bitmap & territories = bitmaps[Territories];
    Bitmap & resources = bitmaps[Resources];

    static sf::Color zoneColors[] =
    {
        Color::Red,
        Color::Green,
        Color::Yellow,
        Color::Magenta,
        Color::Cyan
    };

    extern u32 screenWidth;
    extern u32 screenHeight;

    const Vector2f scale = Vector2f(float(screenWidth) / float(width), float(screenHeight) / float(height));

    for (u32 h = 0; h < height; ++h)
    {
        for (u32 w = 0; w < width; ++w)
        {
            const u32 offset = w + h *width;

            // Heightfield
            const ubyte height = elevation[offset];
            heigthfield.image.setPixel(w, h, Color(height << 4, height << 4, height << 4, 255));

            // Zones
            const ubyte territoryIndex = zones[offset];
            const Territory territory = territoriesInfo[territoryIndex];
            Color territoryColor = Color(0, 0, 0, 255);

            u32 flags = 0;

            switch (territoryBackground)
            {
                case TerritoryBackground::None:
                    break;

                case TerritoryBackground::Territory:
                {
                    territoryColor.a = territoryIndex;

                    if (territory.ocean)
                        flags |= TERRITORIES_FLAGS_WATER;
 
                    flags |= TERRITORIES_FLAGS_VISIBLE;
                }
                break;

                case TerritoryBackground::Biome:
                {
                    u32 biomeIndex = territory.biome;
                    territoryColor.a = biomeIndex;

                    flags |= TERRITORIES_FLAGS_VISIBLE | TERRITORIES_FLAGS_BIOME;
                }
                break;
            }         

            territoryColor.b = flags;
            
            territories.image.setPixel(w, h, territoryColor);

            // Resource
            const u32 poiData = poi[offset];
            Color resColor = Color(0, 0, 0, 0);

            const u32 resourceIndex = poiData & 0xFF;

            if (showLuxuryResources)
            {
                if (resourceIndex >= (u32)LuxuryResource::First && resourceIndex <= (u32)LuxuryResource::Last)
                {
                    const u32 luxuryIndex = resourceIndex - (u32)LuxuryResource::First;

                    if (luxuryResources[luxuryIndex].visible)
                    {
                        resColor = ColorFloat4ToUbyte4(luxuryResources[luxuryIndex].color);

                        Texture & tex = luxuryResourceTextures[luxuryIndex];
                        tex.setSmooth(false);
                        Sprite resSprite;
                        resSprite.setTexture(tex);
                        resSprite.setColor(resColor);
                        resSprite.setOrigin(Vector2f(tex.getSize().x*0.5f, tex.getSize().y*0.5f));
                        resSprite.setPosition(Vector2f((float(w) + 0.5f)*scale.x, (float(h) + 0.5f)*scale.y));
                        resources.sprites.push_back(resSprite);
                    }
                }
            }

            if (showStrategicResources)
            {
                if (resourceIndex >= (u32)StrategicResource::First && resourceIndex <= (u32)StrategicResource::Last)
                {
                    const u32 strategicIndex = resourceIndex - (u32)StrategicResource::First;

                    if (strategicResources[strategicIndex].visible)
                    {
                        resColor = ColorFloat4ToUbyte4(strategicResources[strategicIndex].color);

                        Texture & tex = strategicResourceTextures[strategicIndex];
                        tex.setSmooth(false);
                        Sprite resSprite;
                        resSprite.setTexture(tex);
                        resSprite.setColor(resColor);
                        resSprite.setOrigin(Vector2f(tex.getSize().x*0.5f, tex.getSize().y*0.5f));
                        resSprite.setPosition(Vector2f((float(w) + 0.5f)*scale.x, (float(h) + 0.5f)*scale.y));
                        resources.sprites.push_back(resSprite);
                    }
                }
            }

            resources.image.setPixel(w, h, Color(0, 0, 0, 128));
            resources.drawQuad = true;
            resources.drawSprites = true;
        }
    }

    for (u32 i = 0; i < MapBitmap::Count; ++i)
    {
        auto & bitmap = bitmaps[i];
        bitmap.texture.loadFromImage(bitmap.image);
        bitmap.sprite.setTexture(bitmap.texture);
        bitmap.sprite.setTextureRect(IntRect(0, 0, bitmap.texture.getSize().x, bitmap.texture.getSize().y));
        bitmap.sprite.setScale(scale);

        if (bitmap.quadshader == invalidShaderID)
        {
            switch ((MapBitmap)i)
            {
                default:
                    break;

                case MapBitmap::Territories:
                    bitmap.quadshader = ShaderManager::add("bhkmap/shader/territories_vs.fx", "bhkmap/shader/territories_ps.fx");
                    bitmap.quadblend = sf::BlendMode(BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, BlendMode::Equation::Add);
                break;

                case MapBitmap::Resources:
                    bitmap.quadblend = sf::BlendMode(BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, BlendMode::Equation::Add);
                    break;
            }
        }

        if (bitmap.spriteshader == invalidShaderID)
        {
            switch ((MapBitmap)i)
            {
                default:
                    break;

                case MapBitmap::Resources:
                    bitmap.spriteshader = ShaderManager::add("bhkmap/shader/resources_vs.fx", "bhkmap/shader/resources_ps.fx");
                    bitmap.spriteblend = sf::BlendMode(BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, BlendMode::Equation::Add);
                    break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------
u32 * Map::loadTexture(XMLElement * _xmlTerrainSave, const string & _name)
{
    XMLElement * xmlElevation = _xmlTerrainSave->FirstChildElement((_name + ".Bytes").c_str());
    u32 elevationLength = 0;
    XMLError xmlErr = xmlElevation->QueryUnsignedAttribute("Length", &elevationLength);

    string elevationData = xmlElevation->FirstChild()->ToText()->Value();
    base64::decode_inplace(elevationData);
    std::vector<ubyte> temp;
    temp.resize(elevationLength);
    for (u32 i = 0; i < elevationLength; ++i)
        temp[i] = elevationData[i];

    int x, y, comp;
    u32 * data = (u32*)stbi_load_from_memory(temp.data(), (int)temp.size(), &x, &y, &comp, 0);
    return data;
}

//--------------------------------------------------------------------------------------
void Map::loadMap(const string & _map, const string & _cwd)
{
    // unzip file contents to temp folder
    miniz_cpp::zip_file archive(_map);
    string tempFolder = _cwd + "\\data\\";
    bool created = CreateDirectory(tempFolder.c_str(), nullptr);
    tempFolder = _cwd + "\\data\\zip\\";
    created = CreateDirectory(tempFolder.c_str(), nullptr);
    archive.extractall(tempFolder);

    // load xml
    string xmlSave = tempFolder + "Save.hms";
    tinyxml2::XMLDocument doc;
    XMLError xmlErr = doc.LoadFile(xmlSave.c_str());

    XMLElement * xmlDoc = doc.FirstChildElement("Document");
    XMLElement * xmlTerrainSave = xmlDoc->FirstChildElement("TerrainSave");
    XMLNode * xmlAuthor = xmlTerrainSave->FirstChildElement("Author")->FirstChild();
    if (xmlAuthor)
        author = xmlAuthor->ToText()->Value();

    xmlErr = xmlTerrainSave->FirstChildElement("Width")->ToElement()->QueryUnsignedText(&width);
    xmlErr = xmlTerrainSave->FirstChildElement("Height")->ToElement()->QueryUnsignedText(&height);

    XMLElement * xmlElevation = xmlTerrainSave->FirstChildElement("ElevationTexture.Bytes");
    u32 elevationLength = 0;
    xmlErr = xmlElevation->QueryUnsignedAttribute("Length", &elevationLength);

    const u32 * elevation = loadTexture(xmlTerrainSave, "ElevationTexture");
    const u32 * zones = loadTexture(xmlTerrainSave, "ZonesTexture");
    const u32 * poi = loadTexture(xmlTerrainSave, "POITexture");

    const u32 totalSize = width*height;

    this->elevation.resize(totalSize);
    this->zones.resize(totalSize);
    this->poi.resize(totalSize);

    for (u32 j = 0; j < height; ++j)
    {
        for (u32 i = 0; i < width; ++i)
        {
            const u32 offset = j * width + i;
            this->elevation[offset] = elevation[offset];
            this->zones[offset] = zones[offset];
            this->poi[offset] = poi[offset];
        }
    }

    XMLNode * xmlTerritoryDatabase = xmlTerrainSave->FirstChildElement("TerritoryDatabase");
    XMLElement * xmlTerritories = xmlTerritoryDatabase->FirstChildElement("Territories");
    u32 territoryCount = 0;
    xmlErr = xmlTerritories->QueryUnsignedAttribute("Length", &territoryCount);

    territoriesInfo.clear();
    territoriesInfo.reserve(territoryCount);

    XMLElement * xmlTerritory = xmlTerritories->FirstChildElement("Item")->ToElement();
    while (xmlTerritory)
    {
        Territory territory;

        xmlErr = xmlTerritory->FirstChildElement("ContinentIndex")->ToElement()->QueryUnsignedText(&territory.continent);
        xmlErr = xmlTerritory->FirstChildElement("Biome")->ToElement()->QueryUnsignedText(&territory.biome);
        xmlErr = xmlTerritory->FirstChildElement("IsOcean")->ToElement()->QueryBoolText(&territory.ocean);

        territoriesInfo.push_back(territory);

        xmlTerritory = xmlTerritory->NextSiblingElement("Item");
    }

    loaded = true;

    refresh();
}