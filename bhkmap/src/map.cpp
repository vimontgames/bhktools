#include "map.h"
#include "resourceinfo.h"
#include "maths.h"
#include "tinyxml2.h"
#include "zip_file.hpp"
#include "base64.hpp"

#include "shader/common.h"

#define STBI_ONLY_PNG

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

    if (Vector2u(0,0) == wonderTexture.getSize())
        wonderTexture.loadFromFile("data/img/wonder.png");

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

    extern u32 g_screenWidth;
    extern u32 g_screenHeight;

    const Vector2f scale = Vector2f(float(g_screenWidth) / float(width), float(g_screenHeight) / float(height));

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

            // Landmarks
            const ubyte landmarkIndex = landmarks[offset];

            // Natural Wonders
            const ubyte naturalWonderIndex = naturalwonders[offset];

            switch (territoryBackground)
            {
                case TerritoryBackground::None:
                    break;

                case TerritoryBackground::Territory:
                {
                    territoryColor.r = 0;
                    territoryColor.g = territoryIndex;
                    territoryColor.b = TEXEL_FLAG_VISIBLE;
                    territoryColor.a = territoryIndex;                    
                }
                break;

                case TerritoryBackground::Biome:
                {
                    u32 biomeIndex = territory.biome;

                    territoryColor.r = 0;
                    territoryColor.g = biomeIndex;
                    territoryColor.b = TEXEL_FLAG_VISIBLE;
                    territoryColor.a = territoryIndex;
                }
                break;

                case TerritoryBackground::Landmarks:
                {
                    if (0xFF != landmarkIndex)
                    {
                        // There's a bug in the map editor that makes not all landmarks are actually saved
                        if (landmarkIndex < landmarkInfo.size())
                        {
                            const Landmark landmark = landmarkInfo[landmarkIndex];

                            territoryColor.r = 0;
                            territoryColor.g = landmark.definitonIndex;
                            territoryColor.b = TEXEL_FLAG_VISIBLE;
                            territoryColor.a = landmarkIndex;
                        }
                    }
                }
                break;

                case TerritoryBackground::Wonders:
                {
                    if (0xFF != (naturalWonderIndex & 0xFF))
                    {
                        territoryColor.r = 0;
                        territoryColor.g = naturalWonderIndex;
                        territoryColor.b = TEXEL_FLAG_VISIBLE;
                        territoryColor.a = landmarkIndex;
                    }
                }
                break;
            }    

            if (territory.ocean)
                territoryColor.b |= TEXEL_FLAG_WATER;
            
            territories.image.setPixel(w, h, territoryColor);

            // Resource
            const u32 poiData = poi[offset];
            Color resColor = Color(0, 0, 0, 0);

            const u32 resourceIndex = poiData & 0xFF;
            const u32 wonderIndex = naturalwonders[offset] & 0xFF;

            float cellWidth = float(width) / scale.x;
            Vector2f spriteOffset = Vector2f(0.0f,0.0f);

            if (useHexUVs)
                spriteOffset.x = (h & 1) ? -0.25f * cellWidth : +0.25f * cellWidth;

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
                        resSprite.move(spriteOffset);
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
                        resSprite.move(spriteOffset);
                        resources.sprites.push_back(resSprite);
                    }
                }
            }

            if (showWonders)
            {
                if (0xFF != wonderIndex)
                {
                    resColor = ColorFloat4ToUbyte4(naturalWondersInfo[wonderIndex].color);
                
                    Texture & tex = wonderTexture;
                    tex.setSmooth(false);
                    Sprite resSprite;
                    resSprite.setTexture(tex);
                    resSprite.setColor(resColor);
                    resSprite.setOrigin(Vector2f(tex.getSize().x*0.5f, tex.getSize().y*0.5f));
                    resSprite.setPosition(Vector2f((float(w) + 0.5f)*scale.x, (float(h) + 0.5f)*scale.y));
                    resSprite.move(spriteOffset);
                    resources.sprites.push_back(resSprite);
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
                    bitmap.quadshader = ShaderManager::add("data/shader/territories_vs.fx", "data/shader/territories_ps.fx");
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

                case MapBitmap::Heightfield:
                    bitmap.quadshader = ShaderManager::add("data/shader/heightfield_vs.fx", "data/shader/heightfield_ps.fx");
                    bitmap.quadblend = sf::BlendMode(BlendMode::Factor::One, sf::BlendMode::Factor::Zero, BlendMode::Equation::Add);
                    break;

                case MapBitmap::Resources:
                    bitmap.spriteshader = ShaderManager::add("data/shader/resources_vs.fx", "data/shader/resources_ps.fx");
                    bitmap.spriteblend = sf::BlendMode(BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, BlendMode::Equation::Add);
                    break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------
u32 * Map::loadTexture(XMLElement * _xmlTerrainSave, const string & _name)
{
    XMLElement * xmlBytes = _xmlTerrainSave->FirstChildElement((_name + ".Bytes").c_str());
    u32 elevationLength = 0;
    XMLError xmlErr = xmlBytes->QueryUnsignedAttribute("Length", &elevationLength);

    string base64encodedData = xmlBytes->FirstChild()->ToText()->Value();
    base64::decode_inplace(base64encodedData);
    std::vector<ubyte> temp;
    temp.resize(elevationLength);
    for (u32 i = 0; i < elevationLength; ++i)
        temp[i] = base64encodedData[i];

    int x, y, comp;
    u32 * data = (u32*)stbi_load_from_memory(temp.data(), (int)temp.size(), &x, &y, &comp, 0);
    return data;
}

//--------------------------------------------------------------------------------------
void Map::loadHMap(const string & _map, const string & _cwd)
{
    // unzip file contents to temp folder
    miniz_cpp::zip_file archive(_map);
    string tempFolder = _cwd + "\\tmp\\";
    bool created = CreateDirectory(tempFolder.c_str(), nullptr);
    tempFolder = _cwd + "\\tmp\\zip\\";
    created = CreateDirectory(tempFolder.c_str(), nullptr);
    tempFolder = _cwd + "\\tmp\\zip\\read\\";
    created = CreateDirectory(tempFolder.c_str(), nullptr);
    archive.extractall(tempFolder);

    XMLError xmlErr;

    // load descriptor
    string xmlDescriptorFilename = tempFolder + "Descriptor.hmd";
    xmlErr = xmlDocDescriptor.LoadFile(xmlDescriptorFilename.c_str());

    // load xml
    string xmlSaveFilename = tempFolder + "Save.hms";
    xmlErr = xmlDocSave.LoadFile(xmlSaveFilename.c_str());

    XMLElement * xmlDoc = xmlDocSave.FirstChildElement("Document");
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
    const u32 * landmarks = loadTexture(xmlTerrainSave, "LandmarksTexture");
    const u32 * naturalwonders = loadTexture(xmlTerrainSave, "NaturalWonderTexture");

    const u32 totalSize = width*height;

    this->elevation.resize(totalSize);
    this->zones.resize(totalSize);
    this->poi.resize(totalSize);
    this->landmarks.resize(totalSize);
    this->naturalwonders.resize(totalSize);

    for (u32 j = 0; j < height; ++j)
    {
        for (u32 i = 0; i < width; ++i)
        {
            const u32 offset = j * width + i;
            this->elevation[offset] = elevation[offset];
            this->zones[offset] = zones[offset];
            this->poi[offset] = poi[offset];
            this->landmarks[offset] = landmarks[offset];
            this->naturalwonders[offset] = naturalwonders[offset];
        }
    }

    // load natural wonders info
    {
        XMLElement * xmlNaturalWonderNames = xmlTerrainSave->FirstChildElement("NaturalWonderNames");
        u32 naturalWonderNameCount = 0;
        xmlErr = xmlNaturalWonderNames->QueryUnsignedAttribute("Length", &naturalWonderNameCount);

        naturalWondersInfo.clear();
        naturalWondersInfo.reserve(naturalWonderNameCount);

        XMLNode * xmlWonderNameNode = xmlNaturalWonderNames->FirstChildElement("String");

        int i = 0;

        ColorFloat4 wonderColors[] = 
        {
            {1.00f, 1.00f, 0.50f, 1.00f},
            {0.00f, 1.00f, 1.00f, 1.00f},
            {0.00f, 0.00f, 1.00f, 1.00f},
            {0.00f, 0.60f, 1.00f, 1.00f},
            {0.00f, 0.80f, 1.00f, 1.00f},
            {0.00f, 0.40f, 1.00f, 1.00f},
            {0.00f, 0.20f, 1.00f, 1.00f},
            {0.90f, 0.90f, 0.90f, 1.00f},
            {0.80f, 0.80f, 0.80f, 1.00f},
            {0.70f, 0.70f, 0.70f, 1.00f},
            {0.70f, 0.00f, 0.00f, 1.00f},
            {0.75f, 0.75f, 1.00f, 1.00f},
            {0.90f, 1.00f, 0.60f, 1.00f},
            {1.00f, 1.00f, 0.00f, 1.00f},
        };

        while (xmlWonderNameNode)
        {
            string name = xmlWonderNameNode->FirstChild()->ToText()->Value();
            
            NaturalWonder wonder;

            wonder.name = name;

            wonder.color = wonderColors[i % _countof(wonderColors)];

            naturalWondersInfo.push_back(wonder);

            xmlWonderNameNode = xmlWonderNameNode->NextSiblingElement("String");
            ++i;
        }
    }

    // load territories info
    {
        XMLNode * xmlTerritoryDatabase = xmlTerrainSave->FirstChildElement("TerritoryDatabase");
        XMLElement * xmlTerritories = xmlTerritoryDatabase->FirstChildElement("Territories");
        u32 territoryCount = 0;
        xmlErr = xmlTerritories->QueryUnsignedAttribute("Length", &territoryCount);

        territoriesInfo.clear();
        territoriesInfo.reserve(territoryCount);

        XMLNode * xmlTerritoryNode = xmlTerritories->FirstChildElement("Item");

        if (xmlTerritoryNode)
        {
            XMLElement * xmlTerritory = xmlTerritoryNode->ToElement();
            while (xmlTerritory)
            {
                Territory territory;

                xmlErr = xmlTerritory->FirstChildElement("ContinentIndex")->ToElement()->QueryUnsignedText(&territory.continent);
                xmlErr = xmlTerritory->FirstChildElement("Biome")->ToElement()->QueryUnsignedText(&territory.biome);
                xmlErr = xmlTerritory->FirstChildElement("IsOcean")->ToElement()->QueryBoolText(&territory.ocean);

                territoriesInfo.push_back(territory);

                xmlTerritory = xmlTerritory->NextSiblingElement("Item");
            }
        }
    }

    // load landmarks info
    {
        XMLNode * xmlLandmarkDatabase = xmlTerrainSave->FirstChildElement("LandmarkDatabase");
        XMLElement * xmlLandmarks = xmlLandmarkDatabase->FirstChildElement("Landmarks");
        u32 landmarkCount = 0;
        xmlErr = xmlLandmarks->QueryUnsignedAttribute("Length", &landmarkCount);

        landmarkInfo.clear();
        landmarkInfo.reserve(landmarkCount);

        XMLNode * xmlLandmarkNode = xmlLandmarks->FirstChildElement("Item");

        if (xmlLandmarkNode)
        {
            XMLElement * xmlLandmark = xmlLandmarkNode->ToElement();

            while (xmlLandmark)
            {
                Landmark landmark;

                landmark.name = xmlLandmark->FirstChildElement("Name")->FirstChild()->ToText()->Value();
                xmlErr = xmlLandmark->FirstChildElement("DefinitionIndex")->ToElement()->QueryUnsignedText(&landmark.definitonIndex);

                landmarkInfo.push_back(landmark);

                xmlLandmark = xmlLandmark->NextSiblingElement("Item");
            }
        }
    }

    loaded = true;

    refresh();
}

//--------------------------------------------------------------------------------------
void Map::saveHMap(string & _map, const string & _cwd)
{
    string::size_type idx = _map.rfind('.');
    string extension;
    if (idx != string::npos)
        extension = _map.substr(idx + 1);
    for (auto c : extension)
        c = tolower(c);
    if (extension != "hmap")
        _map += ".hmap";

    XMLElement * xmlDoc = xmlDocSave.FirstChildElement("Document");
    XMLElement * xmlTerrainSave = xmlDoc->FirstChildElement("TerrainSave");

    if (exportLandmarks)
    {
        XMLNode * xmlLandmarkDatabase = xmlTerrainSave->FirstChildElement("LandmarkDatabase");

        if (xmlLandmarkDatabase)
        {
            if (landmarkInfo.empty())
            {

                XMLElement * xmlLandarks = xmlLandmarkDatabase->FirstChildElement("Landmarks");
                xmlLandarks->DeleteAttribute("Length");
                xmlLandarks->DeleteChildren();
                xmlLandarks->SetAttribute("Null", "true");
            }
            else
            {
                assert(!"todo");
            }
        }

        XMLElement * xmlLandmarkBytes = xmlTerrainSave->FirstChildElement("LandmarksTexture.Bytes");

        if (xmlLandmarkBytes)
        {
            // 0xFF0000FF png
            //u32 * pixels = (u32*)malloc(width * height * sizeof(u32));
            //
            //for (u32 j = 0; j < height; ++j)
            //    for (u32 i = 0; i < width; ++i)
            //        pixels[i + j * width] = 0xFF0000FF;

            int len = 0;
            ubyte * emptyPng = stbi_write_png_to_mem((const ubyte*)landmarks.data(), width * sizeof(u32), width, height, 4, &len);
            string emptyPngB64 = base64::encode((char*)emptyPng, (size_t)len);

            xmlLandmarkBytes->SetAttribute("Length", len);
            xmlLandmarkBytes->FirstChild()->SetValue(emptyPngB64.c_str());
        }
    }

    string tempFolder = _cwd + "\\tmp\\";
    bool created = CreateDirectory(tempFolder.c_str(), nullptr);
    tempFolder = _cwd + "\\tmp\\zip\\";
    created = CreateDirectory(tempFolder.c_str(), nullptr);
    tempFolder = _cwd + "\\tmp\\zip\\write\\";
    created = CreateDirectory(tempFolder.c_str(), nullptr);

    string xmlDescriptorFilename = tempFolder + "Descriptor.hmd";
    string xmlSaveFilename = tempFolder + "Save.hms";

    xmlDocDescriptor.SaveFile((xmlDescriptorFilename).c_str());
    xmlDocSave.SaveFile((xmlSaveFilename).c_str());

    miniz_cpp::zip_file archive;
    archive.write(xmlDescriptorFilename.c_str());
    archive.write(xmlSaveFilename.c_str());
    archive.save(_map.c_str());
}

//--------------------------------------------------------------------------------------
// Set all landmarks to 'invalid' 0xFF
//--------------------------------------------------------------------------------------
void Map::clearLandmarks()
{
    landmarkInfo.clear();

    for (u32 j = 0; j < height; ++j)
    {
        for (u32 i = 0; i < width; ++i)
        {
            const u32 offset = i + j * width;
            landmarks[offset] = 0xFF0000FF;
        }
    }
    refresh();
}

//--------------------------------------------------------------------------------------
// Keep only territory 0
//--------------------------------------------------------------------------------------
void Map::clearTerritories()
{
    territoriesInfo.clear();

    Territory empty;
    territoriesInfo.push_back(empty);

    for (u32 j = 0; j < height; ++j)
    {
        for (u32 i = 0; i < width; ++i)
        {
            const u32 offset = i + j * width;
            zones[offset] = 0x00000000;
        }
    }
    refresh();
}