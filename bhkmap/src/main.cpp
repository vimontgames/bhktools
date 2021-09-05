#include "imgui.h" 
#include "imgui-SFML.h" 
#include "ImGuiFileBrowser.h"
#include "Shlobj.h"
#include "stringconvert.h"
#include "guistyle.h"
#include "resourceinfo.h"
#include "map.h"

using namespace std;
using namespace sf;
using namespace ImGui;
using namespace ImGui::SFML;
using namespace imgui_addons;

u32 screenWidth = 1280;
u32 screenHeight = 720;

static ImGuiFileBrowser g_fileDialog;
static string g_myDocumentsPath;
static string g_currentWorkingDirectory;

bool g_openFileDialog = false;

bool g_openDisplayWindow = true;
bool g_openInfoWindow = true;
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

//--------------------------------------------------------------------------------------
int main() 
{
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

    RenderWindow window(VideoMode(screenWidth, screenHeight), "bhkmap 0.2");
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

            if (event.type == Event::Closed) 
                window.close();

            if (event.type == sf::Event::MouseWheelMoved)
            {
                if (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))
                    g_mouseWheelDelta = (float)event.mouseWheel.delta;
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
                if (ImGui::MenuItem("Open .hmap file"))
                    g_openFileDialog = true;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Display"))
                    g_openDisplayWindow = true;

                if (ImGui::MenuItem("Info"))
                    g_openInfoWindow = true;

                ImGui::Separator();

                if (ImGui::MenuItem("Debug"))
                    g_openDebugWindow = true;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("View Help"))
                    g_openHelpWindow = true;

                ImGui::Separator();

                if (ImGui::MenuItem("About bhkmap"))
                    g_openAboutWindow = true;

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (g_openHelpWindow)
        {
            if (Begin("Help", &g_openHelpWindow))
            {
                ImGui::Columns(2, "mycolumns2", false);
                {
                    ImGui::Text("Pan");
                    ImGui::Text("Zoom");
                }
                ImGui::NextColumn();
                {
                    ImGui::Text("Left Mouse Button");
                    ImGui::Text("Ctrl + Mouse Wheel");
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
                }
                ImGui::TreePop();

                ImGui::Columns(1);

                ImGui::TreeNodeEx("View", ImGuiTreeNodeFlags_DefaultOpen);
                {
                    ImGui::Columns(2, "mycolumns2", false);
                    {
                        ImGui::Text("Offset");
                        ImGui::Text("Zoom");
                    }

                    ImGui::NextColumn();
                    {
                        char tmp[256];
                        sprintf_s(tmp, "%.1f, %.1f", g_cameraOffset.x, g_cameraOffset.y);
                        ImGui::Text(tmp);

                        sprintf_s(tmp, "%.1f", g_cameraZoom);
                        ImGui::Text(tmp);
                    }
                }
                ImGui::TreePop();
            }

            ImGui::End();
        }

        if (g_openInfoWindow)
        {
            if (Begin("Info", &g_openInfoWindow))
            {
                ImGui::Columns(2, "mycolumns2", false);  // 2-ways, no border
                {
                    ImGui::Text("Author");
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
                ImGui::Separator();

                ImGui::Columns(2, "mycolumns2", false); 
                {
                    ImGui::Text("Territories");
                }
                ImGui::NextColumn();
                {
                    ImGui::Text("%u", g_map.territoriesInfo.size());
                }
            }

            ImGui::End();
        }

        if (g_openDisplayWindow)
        {
            if (Begin("Display", &g_openDisplayWindow))
            {
                const u32 textLen = 10;

                if (TreeNodeEx("Territories", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    PushItemWidth(80);
                    needRefresh |= Combo("Display", (int*)&g_map.territoryBackground, "None\0Index\0Biome\0\0");
                    PopItemWidth();

                    needRefresh |= Checkbox(getFixedSizeString("Borders", textLen).c_str(), &g_map.showTerritoriesBorders);

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

                            changed |= ImGui::Checkbox(getFixedSizeString(info.name.c_str(), textLen).c_str(), &info.visible);
                            ImGui::SameLine();
                            float * pFloat = (float*)&info.color.rgba[0];
                            changed |= ImGui::ColorEdit4(info.name.c_str(), pFloat, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel);
                        }
                        ImGui::Unindent();

                        return changed;
                    };

                    needRefresh |= ImGui::Checkbox(getFixedSizeString("Strategic", textLen).c_str(), &g_map.showStrategicResources);
                    if (g_map.showStrategicResources)
                        needRefresh |= ListResources(strategicResources, (u32)StrategicResource::First, (u32)StrategicResource::Last);

                    needRefresh |= ImGui::Checkbox(getFixedSizeString("Luxury", textLen).c_str(), &g_map.showLuxuryResources);
                    if (g_map.showLuxuryResources)
                        needRefresh |= ListResources(luxuryResources, (u32)LuxuryResource::First, (u32)LuxuryResource::Last);

                    g_map.bitmaps[Resources].visible = g_map.showStrategicResources || g_map.showLuxuryResources;

                    ImGui::TreePop();
                };
            }
            ImGui::End();
        }

        static bool demo = false;
        if (demo)
            ImGui::ShowDemoWindow(&demo);

        if (g_openFileDialog)
        {
            ImGui::OpenPopup("Open .hmap file");
            g_openFileDialog = false;
            SetCurrentDirectory(g_myDocumentsPath.c_str());
        }

        if (g_fileDialog.showFileDialog("Open .hmap file", ImGuiFileBrowser::DialogMode::OPEN, ImVec2(float(screenWidth)/2.0f, float(screenHeight)/2.0f), ".hmap"))
        {
            SetCurrentDirectory(g_currentWorkingDirectory.c_str());
            const string newFilePath = g_fileDialog.selected_path;
            g_map.path = newFilePath;
            g_map.loadMap(g_map.path, g_currentWorkingDirectory);
            resetCamera();
        }

        if (needRefresh)
            g_map.refresh();

        // update "camera"
        const float panSpeed = 1.0f;
        const float zoomSpeed = 1.1f;

        if (Mouse::isButtonPressed(Mouse::Left))
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
        view.setCenter(sf::Vector2f(screenWidth*0.5f, screenHeight*0.5f));
        view.setSize(sf::Vector2f((float)screenWidth, (float)screenHeight));
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
                    texture.setRepeated(false);
                    
                    auto & sprite = bitmap.sprite;
                    
                    sf::Shader * shader = ShaderManager::get(bitmap.quadshader);
                    
                    RenderStates rs;
                    rs.shader = shader;
                    rs.blendMode = bitmap.quadblend;
                    
                    if (shader)
                    {
                        shader->setUniform("texSize", (Vector2f)texture.getSize());
                        shader->setUniform("screenSize", Vector2f(float(screenWidth), float(screenHeight)));
                        shader->setUniform("borders", g_map.showTerritoriesBorders);
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
                            shader->setUniform("screenSize", Vector2f(float(screenWidth), float(screenHeight)));
                            shader->setUniform("color", Glsl::Vec4(sprite.getColor()));
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
