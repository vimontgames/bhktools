#include "imgui.h" 
#include "imgui-SFML.h" 
#include "ImGuiFileBrowser.h"
#include "Shlobj.h"
#include "stringconvert.h"
#include "guistyle.h"
#include "resourceinfo.h"
#include "map/map.h"

#include "shader/common.h"

using namespace std;
using namespace sf;
using namespace ImGui;
using namespace ImGui::SFML;
using namespace imgui_addons;

bool g_hasFocus = true;
u32 g_screenWidth = 1920;
u32 g_screenHeight = 1024;

static ImGuiFileBrowser g_fileDialog;
static string g_myDocumentsPath;
static string g_currentWorkingDirectory;
static float g_comboxItemWidth = 120;
static const u32 g_fixedTextLengthShort = 10;
static const u32 g_fixedTextLengthLarge = 20;

bool g_openFileDialog = false;
bool g_saveFileDialog = false;
bool g_saveOptionDialog = false;

bool g_openDisplayWindow = true;
bool g_openInfoWindow = true;
bool g_openTerritoriesWindow = true;
bool g_openLandmarksWindow = true;
bool g_openSpawnsWindow = true;
bool g_openWondersWindow = true;
bool g_openDebugWindow = false;

bool g_openHelpWindow = true;
bool g_openAboutWindow = false;

// camera
bool g_cameraPan = false;
Vector2f g_cameraPanOrigin;
Vector2f g_cameraOffset = Vector2f(0, 0);
Vector2f g_cameraPreviousOffset = Vector2f(0, 0);
float g_cameraZoom = 1.0f;
float g_mouseWheelDelta = 0;

static Map g_map;

//--------------------------------------------------------------------------------------
void resetCameraPan()
{
    g_cameraOffset = Vector2f(0, 0);
    g_cameraPreviousOffset = g_cameraOffset;
}

//--------------------------------------------------------------------------------------
void resetCameraZoom()
{
    g_cameraZoom = 1.0f;
}

//--------------------------------------------------------------------------------------
void resetCamera()
{
    resetCameraPan();
    resetCameraZoom();
}

//--------------------------------------------------------------------------------------
class dbg_stream_for_cout : public std::stringbuf
{
public:
    ~dbg_stream_for_cout() { sync(); }
    int sync()
    {
        ::OutputDebugStringA(str().c_str());
        str(std::string()); // Clear the string buffer
        return 0;
    }
};
dbg_stream_for_cout g_DebugStreamFor_cout;

#include "imgui_internal.h"


const char * title = "bhkmap 0.41";

//--------------------------------------------------------------------------------------
int main() 
{
    // init random seed
    srand((u32)time(NULL));

    // Redirect SFML errors to debug output
    std::cout.rdbuf(&g_DebugStreamFor_cout);
    std::streambuf* previous = sf::err().rdbuf(&g_DebugStreamFor_cout);

    // save dir
    TCHAR cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    g_currentWorkingDirectory = string(cwd);

    // default path
    WCHAR userFolder[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, CSIDL_MYDOCUMENTS, userFolder)))
        g_myDocumentsPath = ws2s(wstring(userFolder)) + "\\Humankind\\Maps";

    RenderWindow window(VideoMode(g_screenWidth, g_screenHeight), title, Style::Titlebar | Style::Resize | Style::Close);
    window.setFramerateLimit(60);
    Init(window);

    ShaderManager::init();

    // Setup dear ImGui
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingTransparentPayload = true;

    SetupImGuiStyle();

    Clock deltaClock;
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ProcessEvent(event);

            switch (event.type)
            {
                case Event::Closed:
                    window.close();
                    break;

                case Event::GainedFocus:
                    g_hasFocus = true;
                    break;

                case Event::LostFocus:
                    g_hasFocus = false;
                    break;

                case Event::MouseWheelMoved:
                    if (!IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !IsWindowFocused(ImGuiHoveredFlags_AnyWindow) && g_hasFocus)
                        g_mouseWheelDelta = (float)event.mouseWheel.delta;
                    break;

                case Event::Resized:
                {
                    Vector2f offset = Vector2f(float(g_screenWidth / 2) - float(event.size.width / 2), float(g_screenHeight / 2) - float(event.size.height / 2));
                    g_cameraOffset += offset;
                    g_cameraPreviousOffset += offset;

                    g_cameraZoom = g_cameraZoom / (float(event.size.height) / float(g_screenHeight));

                    g_screenWidth = event.size.width;
                    g_screenHeight = event.size.height;
                }
                break;
            }
        }

        Update(window, deltaClock.restart());

        bool showUI = true;

        ImGuiViewport * viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_None);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &showUI, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground);
        ImGui::PopStyleVar(3);
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);   

        bool needRefresh = false;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import"))
                    g_openFileDialog = true;

                if(ImGui::MenuItem("Export"))
                    g_saveFileDialog = true;

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    window.close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Display", nullptr, g_openDisplayWindow))
                    g_openDisplayWindow ^= 1;

                if (ImGui::MenuItem("Infos",nullptr, g_openInfoWindow))
                    g_openInfoWindow ^= 1;

                if (ImGui::MenuItem("Landmarks", nullptr, g_openLandmarksWindow))
                    g_openLandmarksWindow ^= 1;

                if (ImGui::MenuItem("Landmarks", nullptr, g_openSpawnsWindow))
                    g_openSpawnsWindow ^= 1;

                if (ImGui::MenuItem("Territories", nullptr, g_openTerritoriesWindow))
                    g_openTerritoriesWindow ^= 1;

                //if (ImGui::MenuItem("Wonders", nullptr, g_openWondersWindow))
                //    g_openWondersWindow ^= 1;

                ImGui::Separator();

                if (ImGui::MenuItem("Help", nullptr, g_openHelpWindow))
                    g_openHelpWindow ^= 1;

                if (ImGui::MenuItem("Debug", nullptr, g_openDebugWindow))
                    g_openDebugWindow ^= 1;

                ImGui::EndMenu();
            }

            //if (ImGui::BeginMenu("About"))
            //{
            //    if (ImGui::MenuItem("bhkmap", nullptr, g_openAboutWindow))
            //        g_openAboutWindow ^= 1;
            //
            //    ImGui::EndMenu();
            //}

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (g_openHelpWindow)
        {
            if (Begin("Help", &g_openHelpWindow))
            {
                ImGui::Columns(2, "mycolumns2", false);
                {
                    SetColumnWidth(0, 96.0f);

                    ImGui::Text("Pan");
                    ImGui::Text("Zoom");
                }
                ImGui::NextColumn();
                {
                    ImGui::Text("Left Mouse Button");
                    ImGui::Text("Mouse Wheel");
                }             
            }
            ImGui::End();
        }

        if (g_openDebugWindow)
        {
            if (Begin("Debug", &g_openDebugWindow))
            {
                ImGui::TreeNodeEx("Mouse", ImGuiTreeNodeFlags_DefaultOpen);
                {
                    ImGui::Columns(2, "mycolumns2", false);
                    {
                        ImGui::Text("Position");
                        ImGui::Text("Buttons");
                        ImGui::Text("Wheel");
                    }

                    ImGui::NextColumn();
                    {
                        const auto & mousePos = sf::Mouse::getPosition(window);
                        ImGui::Text((to_string(mousePos.x) + ", " + to_string(mousePos.y)).c_str());

                        string mouseButtonsString = "";
                        if (sf::Mouse::isButtonPressed(Mouse::Left))
                            mouseButtonsString += "Left ";

                        if (sf::Mouse::isButtonPressed(Mouse::Middle))
                            mouseButtonsString += "Middle ";

                        if (sf::Mouse::isButtonPressed(Mouse::Right))
                            mouseButtonsString += "Right ";

                        ImGui::Text(mouseButtonsString.c_str());

                        ImGui::Text((to_string((int)g_mouseWheelDelta)).c_str());
                    }

                    ImGui::TreePop();
                }

                ImGui::Columns(1);

                if (TreeNodeEx("View", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    Columns(2, "mycolumns2", false);
                    {
                        ImGui::Text("HasFocus");
                        ImGui::Text("Offset");
                        ImGui::Text("Zoom");
                    }

                    NextColumn();
                    {
                        ImGui::Text(g_hasFocus ? "true" : "false");

                        char tmp[256];
                        sprintf_s(tmp, "%.1f, %.1f", g_cameraOffset.x, g_cameraOffset.y);
                        ImGui::Text(tmp);

                        sprintf_s(tmp, "%.1f", g_cameraZoom);
                        ImGui::Text(tmp);
                    }

                    TreePop();
                }
            }

            ImGui::End();
        }

        if (g_openInfoWindow)
        {
            if (Begin("Infos", &g_openInfoWindow))
            {
                ImGui::Columns(2, "mycolumns2", false);  // 2-ways, no border
                {
                    SetColumnWidth(0, 96.0f);
                    ImGui::Text("Author");;
                    ImGui::Text("Width");
                    ImGui::Text("Height");
                }
                ImGui::NextColumn();
                {
                    ImGui::Text(g_map.author.c_str());
                    ImGui::Text(to_string(g_map.width).c_str());
                    ImGui::Text(to_string(g_map.height).c_str());
                }

                ImGui::Columns(1);

                if (TreeNodeEx("Options", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox(getFixedSizeString("Map cycling", g_fixedTextLengthShort).c_str(), &g_map.useMapCycling);
                    ImGui::Checkbox(getFixedSizeString("Procedural mountain chains", g_fixedTextLengthShort).c_str(), &g_map.useProceduralMountainChains);

                    TreePop();
                }
            }

            ImGui::End();
        }

        if (g_openTerritoriesWindow)
        {
            if (Begin("Territories", &g_openTerritoriesWindow))
            {
                if (ImGui::Button(("Remove all territories (" + to_string(g_map.territoriesInfo.size()) + ")").c_str()))
                    g_map.clearTerritories();

                for (u32 i = 0; i < g_map.territoriesInfo.size(); ++i)
                {
                    if (TreeNodeEx(to_string(i).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        PushItemWidth(g_comboxItemWidth);

                        auto & territory = g_map.territoriesInfo[i];
                        
                        needRefresh |= Combo("Continent", (int*)&territory.continent, "Ocean\0Continent 1\0Continent 2\0Continent 3\0Continent 4\0Continent 5\0Continent 6\0Continent 7\0\0");
                        needRefresh |= Combo("Biome", (int*)&territory.biome, "Arctic\0Badlands\0Desert\0Grassland\0Meditarranean\0Savanna\0Taiga\0Temperate\0Tropical\0Tundra\0\0");

                        territory.ocean = (territory.continent == 0) ? true : false;
                        PopItemWidth();

                        TreePop();
                    }
                }
            }
            ImGui::End();
        }

        if (g_openSpawnsWindow)
        {
            if (Begin("Spawns", &g_openSpawnsWindow))
            {
                if (ImGui::Button("Randomize order"))
                    g_map.randomizeSpawnOrder();

                for (u32 i = 0; i < g_map.allSpawnsPoints.size(); ++i)
                {
                    SpawnPoint & spawn = g_map.allSpawnsPoints[i];

                    if (spawn.flags & (1 << (g_map.spawnPlayerCountDisplayed-1)))
                    {
                        if (TreeNodeEx(("Player " + to_string(spawn.index[g_map.spawnPlayerCountDisplayed - 1]+1)).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            PushItemWidth(g_comboxItemWidth);

                            needRefresh |= ImGui::InputInt2("Position", (int*)&spawn.pos);

                            if (BeginCombo("Flags", to_string(spawn.flags).c_str()))
                            {
                                bool selectedFlags[_countof(g_map.spawnInfo)];
                                for (u32 b = 0; b < _countof(g_map.spawnInfo); ++b)
                                {
                                    if (spawn.flags & (1 << b))
                                        selectedFlags[b] = true;
                                    else
                                        selectedFlags[b] = false;

                                    needRefresh |= ImGui::Checkbox((to_string(b + 1) + " player" + string(b ? "s" : "")).c_str(), &selectedFlags[b]);

                                    if (selectedFlags[b])
                                        spawn.flags |= 1 << b;
                                    else
                                        spawn.flags &= ~(1 << b);
                                }

                                EndCombo();
                            }

                            PopItemWidth();

                            TreePop();
                        }
                    }
                }
            }
            ImGui::End();
        }

        if (g_openLandmarksWindow)
        {
            if (Begin("Landmarks", &g_openLandmarksWindow))
            {
                if (ImGui::Button( ("Remove all landmarks (" + to_string(g_map.landmarkInfo.size()) + ")").c_str()))
                    g_map.clearLandmarks();

                for (u32 i = 0; i < g_map.landmarkInfo.size(); ++i)
                {
                    if (TreeNodeEx(to_string(i).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        PushItemWidth(g_comboxItemWidth);

                        auto & landmark = g_map.landmarkInfo[i];

                        char buffer[1024];
                        sprintf_s(buffer, landmark.name.c_str());
                        bool changed = ImGui::InputText("Name", buffer, 1024);
                        if (changed)
                        {
                            landmark.name = buffer;
                            needRefresh = true;
                        }
                        
                        needRefresh |= Combo("Definition", (int*)&landmark.definitonIndex, "Desert\0Forest\0Lake\0Mountain\0River\0\0");

                        PopItemWidth();
                        TreePop();
                    }
                }
            }
            ImGui::End();
        }

        //if (g_openWondersWindow)
        //{
        //    if (Begin("Wonders", &g_openWondersWindow))
        //    {
        //
        //    }
        //    ImGui::End();
        //}

        if (g_openDisplayWindow)
        {
            if (Begin("Display", &g_openDisplayWindow))
            {
                if (TreeNodeEx("Map", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    PushItemWidth(g_comboxItemWidth);
                    needRefresh |= Combo("Filter", (int*)&g_map.territoryBackground, "None\0Tile\0Territories\0Biomes\0Landmarks\0Wonders\0\0");
                    PopItemWidth();

                    needRefresh |= Checkbox(getFixedSizeString("Borders", g_fixedTextLengthShort).c_str(), &g_map.showTerritoriesBorders);

                    ImGui::SameLine();
                    needRefresh |= Checkbox(getFixedSizeString("Hexes", g_fixedTextLengthShort).c_str(), &g_map.useHexUVs);


                    g_map.bitmaps[Territories].visible = g_map.territoryBackground != TerritoryBackground::None;

                    TreePop();
                }

                if (ImGui::TreeNodeEx("Resources", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    auto ListResources = [=](ResourceInfo * _infos, u32 _first, u32 _last)
                    {
                        bool changed = false;

                        ImGui::Indent();
                        for (u32 i = _first; i <= _last; ++i)
                        {
                            const u32 index = i - _first;
                            auto & info = _infos[index];

                            changed |= ImGui::Checkbox(getFixedSizeString(info.name.c_str(), g_fixedTextLengthLarge).c_str(), &info.visible);
                            ImGui::SameLine();
                            float * pFloat = (float*)&info.color.rgba[0];
                            changed |= ImGui::ColorEdit4(info.name.c_str(), pFloat, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel);

                            ImGui::SameLine();
                            ImGui::Text(to_string(_infos[index].count).c_str());
                        }
                        ImGui::Unindent();

                        return changed;
                    };

                    needRefresh |= ImGui::Checkbox(getFixedSizeString("Strategic", g_fixedTextLengthShort).c_str(), &g_map.showStrategicResources);
                    if (g_map.showStrategicResources)
                        needRefresh |= ListResources(strategicResources, (u32)StrategicResource::First, (u32)StrategicResource::Last);

                    needRefresh |= ImGui::Checkbox(getFixedSizeString("Luxury", g_fixedTextLengthShort).c_str(), &g_map.showLuxuryResources);
                    if (g_map.showLuxuryResources)
                        needRefresh |= ListResources(luxuryResources, (u32)LuxuryResource::First, (u32)LuxuryResource::Last);

                    needRefresh |= ImGui::Checkbox(getFixedSizeString("Wonders", g_fixedTextLengthShort).c_str(), &g_map.showWonders);
                    if (g_map.showWonders)
                        needRefresh |= ListResources(naturalWonderResources, (u32)StrategicResource::First, (u32)StrategicResource::Last);

                    needRefresh |= ImGui::Checkbox(getFixedSizeString("Spawns", g_fixedTextLengthShort).c_str(), &g_map.showSpawnPoints);
                    if (g_map.showSpawnPoints)
                    {
                        PushItemWidth(g_comboxItemWidth);

                        ImGui::Indent();
                        {
                            for (u32 i = 0; i < _countof(g_map.spawnInfo); ++i)
                            {
                                needRefresh |= ImGui::RadioButton(getFixedSizeString(to_string(i + 1) + " player" + string(i ? "s" : ""), g_fixedTextLengthLarge).c_str(), (int*)&g_map.spawnPlayerCountDisplayed, int(i + 1));
                                SameLine();

                                u32 count = 0;
                                for (u32 j = 0; j < g_map.allSpawnsPoints.size(); ++j)
                                {
                                    const SpawnPoint & spawn = g_map.allSpawnsPoints[j];
                                    if (spawn.flags & (1 << i))
                                        count++;
                                }

                                ImGui::Text( "%u/%u", count, i + 1);
                            }
                        }
                        ImGui::Unindent();
                        
                        PopItemWidth();
                    }


                    g_map.bitmaps[Resources].visible = g_map.showStrategicResources || g_map.showLuxuryResources || g_map.showWonders || g_map.showSpawnPoints;

                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }

        if (g_saveOptionDialog)
        {
            ImGui::OpenPopup("Export options");

            if (ImGui::BeginPopupModal("Export options", &g_saveOptionDialog))
            {
                PushTextWrapPos();

                ImGui::Text("Unselected categories will not export the changes made in bhkmap and will keep their original values.");
                ImGui::Text("\n");

                ImGui::Checkbox(getFixedSizeString("Infos", g_fixedTextLengthShort).c_str(), &g_map.overrideMapOptions);
                ImGui::Checkbox(getFixedSizeString("Spawns", g_fixedTextLengthShort).c_str(), &g_map.overridePlayerSpawns);
                ImGui::Checkbox(getFixedSizeString("Landmarks", g_fixedTextLengthShort).c_str(), &g_map.overrideLandmarks);
                ImGui::Text("\n");  
                
                if (ImGui::Button("Export"))
                {
                    g_map.exportHMAP(g_map.path, g_currentWorkingDirectory);
                    g_saveOptionDialog = false;
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel"))
                    g_saveOptionDialog = false;

                PopTextWrapPos();

                ImGui::EndPopup();
            }
        }

        static bool demo = false;
        if (demo)
            ImGui::ShowDemoWindow(&demo);

        if (g_openFileDialog)
        {
            ImGui::OpenPopup("Import");
            g_openFileDialog = false;
            SetCurrentDirectory(g_myDocumentsPath.c_str());
        }
        else if (g_saveFileDialog)
        {
            ImGui::OpenPopup("Export");
            g_saveFileDialog = false;
            SetCurrentDirectory(g_myDocumentsPath.c_str());
        }

        if (g_fileDialog.showFileDialog("Import", ImGuiFileBrowser::DialogMode::OPEN, ImVec2(float(g_screenWidth)/2.0f, float(g_screenHeight)/2.0f), ".hmap"))
        {
            const string newFilePath = g_fileDialog.selected_path;
            g_map.path = newFilePath;
            
            if (g_map.importHMAP(g_map.path, g_currentWorkingDirectory))
            {
                SetCurrentDirectory(g_currentWorkingDirectory.c_str());
                g_map.refresh();

                window.setTitle(string(title + string(" - ") + g_map.path).c_str());
                resetCamera();
            }
        }
        else if (g_fileDialog.showFileDialog("Export", ImGuiFileBrowser::DialogMode::SAVE, ImVec2(float(g_screenWidth) / 2.0f, float(g_screenHeight) / 2.0f), ".hmap"))
        {
            const string newFilePath = g_fileDialog.selected_path;
            g_map.path = newFilePath;
            g_saveOptionDialog = true;
        }

        if (needRefresh)
            g_map.refresh();

        // update "camera"
        const float panSpeed = 1.0f;
        const float zoomSpeed = 1.1f;

        if (Mouse::isButtonPressed(Mouse::Left) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !IsWindowFocused(ImGuiHoveredFlags_AnyWindow) && g_hasFocus)
        {
            if (!g_cameraPan)
            {
                // begin pan
                g_cameraPanOrigin = (Vector2f)Mouse::getPosition(window);
                g_cameraPan = true;
            }
            else
            {
                //continue pan
                g_cameraOffset = (g_cameraPanOrigin - (Vector2f)Mouse::getPosition(window)) * panSpeed * (g_cameraZoom*g_cameraZoom) + g_cameraPreviousOffset;
            }
        }
        else if (g_cameraPan)
        {
            // end pan
            g_cameraPan = false;
            g_cameraPreviousOffset = g_cameraOffset;
        }

        if (0 != g_mouseWheelDelta)
        {
            if (g_mouseWheelDelta < 0)
                g_cameraZoom *= zoomSpeed;
            else if (g_mouseWheelDelta > 0)
                g_cameraZoom /= zoomSpeed;

            g_mouseWheelDelta = 0;
        }

        window.clear();

        sf::View view;
        view.setCenter(sf::Vector2f(g_screenWidth*0.5f, g_screenHeight*0.5f));
        view.setSize(sf::Vector2f((float)g_screenWidth, (float)g_screenHeight));
        view.zoom(g_cameraZoom); // zeng

        view.move(g_cameraOffset);

        window.setView(view);

        for (u32 i = 0; i < MapBitmap::Count; ++i)
        {
            auto & bitmap = g_map.bitmaps[i];
            if (bitmap.visible)
            {
                if (bitmap.drawQuad)
                {
                    auto & texture = bitmap.texture;
                    texture.setRepeated(true);
                    
                    auto & sprite = bitmap.sprite;
                    
                    sf::Shader * shader = ShaderManager::get(bitmap.quadshader);
                    
                    RenderStates rs;
                    rs.shader = shader;
                    rs.blendMode = bitmap.quadblend;
                    
                    if (shader)
                    {
                        shader->setUniform("texSize", (Vector2f)texture.getSize());
                        shader->setUniform("screenSize", Vector2f(float(g_screenWidth), float(g_screenHeight)));

                        int passFlags = 0;

                        switch (g_map.territoryBackground)
                        {
                            default:
                            case TerritoryBackground::None:
                            break;

                            case TerritoryBackground::Tile:
                                passFlags = PASS_TYPE_TILE;
                                break;

                            case TerritoryBackground::Territory:
                                passFlags = PASS_TYPE_TERRITORY;
                            break;

                            case TerritoryBackground::Biome:
                                passFlags = PASS_TYPE_BIOME;
                            break;

                            case TerritoryBackground::Landmarks:
                                passFlags = PASS_TYPE_LANDMARK;
                            break;

                            case TerritoryBackground::Wonders:
                                passFlags = PASS_TYPE_WONDER;
                            break;
                        }

                        if (g_map.useHexUVs)
                            passFlags |= PASS_FLAG_HEXES;

                        if (g_map.showTerritoriesBorders)
                            passFlags |= PASS_FLAG_BORDERS;

                        shader->setUniform("passFlags", passFlags);
                    }
                    
                    window.draw(sprite, rs);
                }

                if (bitmap.drawSprites)
                {
                    sf::Shader * shader = ShaderManager::get(bitmap.spriteshader);

                    RenderStates rs;
                    rs.shader = shader;
                    rs.blendMode = bitmap.spriteblend;

                    for (u32 j = 0; j < bitmap.sprites.size(); ++j)
                    {
                        Sprite & sprite = bitmap.sprites[j];

                        if (shader)
                        {
                            shader->setUniform("texSize", (Vector2f)sprite.getTexture()->getSize());
                            shader->setUniform("screenSize", Vector2f(float(g_screenWidth), float(g_screenHeight)));

                            auto color = Glsl::Vec4(sprite.getColor());
                            shader->setUniform("color", color);
                        }

                        window.draw(sprite, rs);
                    }
                }
            }
        }

        Render(window);
        window.display();

        if (sf::Keyboard::isKeyPressed(Keyboard::F6))
        {
            SetCurrentDirectory(g_currentWorkingDirectory.c_str());
            ShaderManager::update();
        }
    }

    ShaderManager::deinit();
    Shutdown();

    return 0;
}
