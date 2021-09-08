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

#include "map_import.hpp"
#include "map_export.hpp"
#include "map_refresh.hpp"
#include "map_actions.hpp"

//--------------------------------------------------------------------------------------
void Map::loadIcons()
{
    // load resource textures if needed
    for (u32 i = 0; i < (u32)StrategicResource::Count; ++i)
    {
        ResourceInfo & shared = strategicResources[i];
        Texture & tex = shared.texture;
        if (Vector2u(0, 0) == tex.getSize())
            tex.loadFromFile("data/img/" + strategicResources[i].name + ".png");
        shared.count = 0;
    }

    for (u32 i = 0; i < (u32)LuxuryResource::Count; ++i)
    {
        ResourceInfo & shared = luxuryResources[i];
        Texture & tex = shared.texture;
        if (Vector2u(0, 0) == tex.getSize())
            tex.loadFromFile("data/img/" + luxuryResources[i].name + ".png");
        shared.count = 0;
    }

    for (u32 i = 0; i < (u32)NaturalWonderResource::Count; ++i)
    {
        ResourceInfo & shared = naturalWonderResources[i];
        Texture & tex = shared.texture;
        if (Vector2u(0, 0) == tex.getSize())
            tex.loadFromFile("data/img/wonder.png");
        shared.count = 0;
    }

    for (u32 i = 0; i < MapBitmap::Count; ++i)
    {
        auto & bitmap = bitmaps[i];
        bitmap.image.create(width, height);
        bitmap.sprites.clear();
    }

    for (u32 i = 0; i < _countof(spawnInfo); ++i)
    {
        auto & info = spawnInfo[i];
        if (Vector2u(0, 0) == info.texture.getSize())
            info.texture.loadFromFile("data/img/" + to_string(i + 1) + ".png");  
    }
}