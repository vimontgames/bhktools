//--------------------------------------------------------------------------------------
// Change spawn order
//--------------------------------------------------------------------------------------
void Map::randomizeSpawnOrder()
{
    const u32 count = (u32)allSpawnsPoints.size();
    const u32 players = spawnPlayerCountDisplayed;

    if (count)
    {
        const u32 mask = 1 << (players - 1);

        for (u32 i = 0; i < 256; ++i)
        {
            const u32 randSrc = rand() % count;
            const u32 randDst = rand() % count;

            if (randDst != randSrc)
            {
                if (allSpawnsPoints[randSrc].flags & allSpawnsPoints[randDst].flags & mask)
                {
                    SpawnPoint tmp = allSpawnsPoints[randSrc];
                    allSpawnsPoints[randSrc] = allSpawnsPoints[randDst];
                    allSpawnsPoints[randDst] = tmp;
                }
            }
        }
    }

    computeSpawnOrder();
    refresh();
}

//--------------------------------------------------------------------------------------
// compute spawn index according to array order
//--------------------------------------------------------------------------------------
void Map::computeSpawnOrder()
{
    u32 curPlayerIndex[_countof(spawnInfo)];
    memset(curPlayerIndex, 0x0, sizeof(curPlayerIndex));

    for (u32 i = 0; i < _countof(spawnInfo); ++i)
    {
        for (u32 j = 0; j < allSpawnsPoints.size(); ++j)
        {
            SpawnPoint & spawn = allSpawnsPoints[j];
            if ((1 << i) & spawn.flags)
            {
                spawn.fileIndex = curPlayerIndex[i];
                curPlayerIndex[i]++;
            }
        }
    }
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