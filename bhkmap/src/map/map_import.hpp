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
bool Map::importHMAP(const string & _map, const string & _cwd)
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

    xmlErr = xmlTerrainSave->FirstChildElement("UseMapCycling")->ToElement()->QueryBoolText(&useMapCycling);
    xmlErr = xmlTerrainSave->FirstChildElement("UseProceduralMountainChains")->ToElement()->QueryBoolText(&useProceduralMountainChains);

    XMLElement * xmlElevation = xmlTerrainSave->FirstChildElement("ElevationTexture.Bytes");
    u32 elevationLength = 0;
    xmlErr = xmlElevation->QueryUnsignedAttribute("Length", &elevationLength);

    const u32 * elevation = loadTexture(xmlTerrainSave, "ElevationTexture");
    const u32 * zones = loadTexture(xmlTerrainSave, "ZonesTexture");
    const u32 * poi = loadTexture(xmlTerrainSave, "POITexture");
    const u32 * landmarks = loadTexture(xmlTerrainSave, "LandmarksTexture");
    const u32 * naturalwonders = loadTexture(xmlTerrainSave, "NaturalWonderTexture");

    const u32 totalSize = width * height;

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

    // load spawn points
    {
        XMLElement * xmlEntitiesProvider = xmlTerrainSave->FirstChildElement("EntitiesProvider");
        XMLElement * xmlSpawnPoints = xmlEntitiesProvider->FirstChildElement("SpawnPoints");

        u32 spawnPointCount = 0;
        xmlErr = xmlSpawnPoints->QueryUnsignedAttribute("Length", &spawnPointCount);

        allSpawnsPoints.clear();
        allSpawnsPoints.reserve(spawnPointCount);

        XMLNode * xmlSpawnNode = xmlSpawnPoints->FirstChildElement("Item");

        if (xmlSpawnNode)
        {
            XMLElement * xmlSpawn = xmlSpawnNode->ToElement();
            while (xmlSpawn)
            {
                SpawnPoint spawn;

                xmlErr = xmlSpawn->FirstChildElement("SpawnPoints")->FirstChildElement("Column")->ToElement()->QueryUnsignedText(&spawn.pos.x);
                xmlErr = xmlSpawn->FirstChildElement("SpawnPoints")->FirstChildElement("Row")->ToElement()->QueryUnsignedText(&spawn.pos.y);

                spawn.pos.y = height - spawn.pos.y - 1;

                xmlErr = xmlSpawn->FirstChildElement("Flags")->ToElement()->QueryUnsignedText(&spawn.flags);

                allSpawnsPoints.push_back(spawn);

                xmlSpawn = xmlSpawn->NextSiblingElement("Item");
            }
        }

        // default to max spawn count
        for (u32 i = 0; i < _countof(spawnInfo); ++i)
        {
            for (u32 j = 0; j < allSpawnsPoints.size(); ++j)
            {
                SpawnPoint & spawn = allSpawnsPoints[j];
                if ((1 << i) & spawn.flags)
                    spawnPlayerCountDisplayed = i + 1;
            }
        }

        computeSpawnOrder();
    }

    // load natural wonders info
    {
        //XMLElement * xmlNaturalWonderNames = xmlTerrainSave->FirstChildElement("NaturalWonderNames");
        //u32 naturalWonderNameCount = 0;
        //xmlErr = xmlNaturalWonderNames->QueryUnsignedAttribute("Length", &naturalWonderNameCount);
        //
        //naturalWondersInfo.clear();
        //naturalWondersInfo.reserve(naturalWonderNameCount);
        //
        //XMLNode * xmlWonderNameNode = xmlNaturalWonderNames->FirstChildElement("String");
        //
        //int i = 0;
        //
        //ColorFloat4 wonderColors[] = 
        //{
        //    {1.00f, 1.00f, 0.50f, 1.00f},
        //    {0.00f, 1.00f, 1.00f, 1.00f},
        //    {0.00f, 0.00f, 1.00f, 1.00f},
        //    {0.00f, 0.60f, 1.00f, 1.00f},
        //    {0.00f, 0.80f, 1.00f, 1.00f},
        //    {0.00f, 0.40f, 1.00f, 1.00f},
        //    {0.00f, 0.20f, 1.00f, 1.00f},
        //    {0.90f, 0.90f, 0.90f, 1.00f},
        //    {0.80f, 0.80f, 0.80f, 1.00f},
        //    {0.70f, 0.70f, 0.70f, 1.00f},
        //    {0.70f, 0.00f, 0.00f, 1.00f},
        //    {0.75f, 0.75f, 1.00f, 1.00f},
        //    {0.90f, 1.00f, 0.60f, 1.00f},
        //    {1.00f, 1.00f, 0.00f, 1.00f},
        //};
        //
        //while (xmlWonderNameNode)
        //{
        //    string name = xmlWonderNameNode->FirstChild()->ToText()->Value();
        //    
        //    NaturalWonder wonder;
        //
        //    wonder.name = name;
        //
        //    wonder.color = wonderColors[i % _countof(wonderColors)];
        //
        //    naturalWondersInfo.push_back(wonder);
        //
        //    xmlWonderNameNode = xmlWonderNameNode->NextSiblingElement("String");
        //    ++i;
        //}
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

    return true;
}