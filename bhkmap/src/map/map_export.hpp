//--------------------------------------------------------------------------------------
void Map::exportHMAP(string & _map, const string & _cwd)
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

    if (overrideMapOptions)
    {
        xmlTerrainSave->FirstChildElement("UseMapCycling")->FirstChild()->SetValue(useMapCycling ? "true" : "false");
        xmlTerrainSave->FirstChildElement("UseProceduralMountainChains")->FirstChild()->SetValue(useProceduralMountainChains ? "true" : "false");
    }

    if (overridePlayerSpawns)
    {
        XMLElement * xmlEntitiesProvider = xmlTerrainSave->FirstChildElement("EntitiesProvider");
        XMLElement * xmlSpawnPoints = xmlEntitiesProvider->FirstChildElement("SpawnPoints");

        xmlSpawnPoints->DeleteChildren();

        xmlSpawnPoints->SetAttribute("Length", allSpawnsPoints.size());

        for (u32 i = 0; i < allSpawnsPoints.size(); ++i)
        {
            const auto & spawn = allSpawnsPoints[i];

            XMLElement * xmlItem = xmlSpawnPoints->InsertNewChildElement("Item");
            
            XMLElement * xmlSpawnPoints = xmlItem->InsertNewChildElement("SpawnPoints");
            xmlSpawnPoints->InsertNewChildElement("Column")->InsertFirstChild(xmlDocSave.NewText(to_string(spawn.pos.x).c_str()));
            xmlSpawnPoints->InsertNewChildElement("Row")->InsertFirstChild(xmlDocSave.NewText(to_string(height - spawn.pos.y - 1).c_str()));

            XMLElement * xmlFlags = xmlItem->InsertNewChildElement("Flags");
            xmlFlags->InsertFirstChild(xmlDocSave.NewText(to_string(spawn.flags).c_str()));
        }        
    }

    if (overrideLandmarks)
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