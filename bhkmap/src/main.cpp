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

static ImGuiFileBrowser fileDialog;
static string myDocumentsPath;
static bool openFileDialog = false;
static string currentWorkingDirectory;

static Map map;

//--------------------------------------------------------------------------------------
int main() 
{
    // save dir
    TCHAR cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    currentWorkingDirectory = string(cwd);

    // default path
    WCHAR userFolder[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, CSIDL_MYDOCUMENTS, userFolder)))
        myDocumentsPath = ws2s(wstring(userFolder)) + "\\Humankind\\Maps";

    RenderWindow window(VideoMode(screenWidth, screenHeight), "bhkmap 0.1");
    window.setFramerateLimit(60);
    Init(window);

    // Setup dear ImGui
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingTransparentPayload = true;

    FiftyShadesOfGreyStyle();

    Clock deltaClock;
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ProcessEvent(event);

            if (event.type == Event::Closed) 
                window.close();
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
                    openFileDialog = true;
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About bhkmap"))
                    __noop;
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (Begin("Info"))
        {
            ImGui::Columns(2, "mycolumns2", false);  // 2-ways, no border
            {
                ImGui::Text("Author");
                ImGui::Text("Width");
                ImGui::Text("Height");
            }
            ImGui::NextColumn();
            {
                ImGui::Text(map.author.c_str());
                ImGui::Text(to_string(map.width).c_str());
                ImGui::Text(to_string(map.height).c_str());
            }
        }
        ImGui::End();

        if (Begin("Display"))
        {
            const u32 textLen = 10;

            if (ImGui::TreeNodeEx("Territories", ImGuiTreeNodeFlags_DefaultOpen))
            {
                needRefresh |= ImGui::Checkbox(getFixedSizeString("Ocean", textLen).c_str(), &map.showOceanTerritories);
                ImGui::SameLine();
                needRefresh |= ImGui::Checkbox(getFixedSizeString("Land", textLen).c_str(), &map.showLandTerritories);

                map.bitmaps[Territories].visible = map.showLandTerritories || map.showOceanTerritories;

                ImGui::TreePop();
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
                
                needRefresh |= ImGui::Checkbox(getFixedSizeString("Strategic", textLen).c_str(), &map.showStrategicResources);
                if (map.showStrategicResources)
                    needRefresh |= ListResources(strategicResources, (u32)StrategicResource::First, (u32)StrategicResource::Last);

                needRefresh |= ImGui::Checkbox(getFixedSizeString("Luxury", textLen).c_str(), &map.showLuxuryResources);
                if (map.showLuxuryResources)
                    needRefresh |= ListResources(luxuryResources, (u32)LuxuryResource::First, (u32)LuxuryResource::Last);

                map.bitmaps[Resources].visible = map.showStrategicResources || map.showLuxuryResources;

                ImGui::TreePop();
            };
        }

        static bool demo = false;
        if (demo)
            ImGui::ShowDemoWindow(&demo);

        ImGui::End();

        if (openFileDialog)
        {
            ImGui::OpenPopup("Open .hmap file");
            openFileDialog = false;
        }

        SetCurrentDirectory(myDocumentsPath.c_str());
        if (fileDialog.showFileDialog("Open .hmap file", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(float(screenWidth)/2.0f, float(screenHeight)/2.0f), ".hmap"))
        {
            const string newFilePath = fileDialog.selected_path;
            map.path = newFilePath;
            map.loadMap(map.path, currentWorkingDirectory);
        }
        SetCurrentDirectory(currentWorkingDirectory.c_str());

        if (needRefresh)
            map.refresh();

        window.clear();

        for (u32 i = 0; i < MapBitmap::Count; ++i)
            if (map.bitmaps[i].visible)
                window.draw(map.bitmaps[i].sprite);

        Render(window);
        window.display();
    }

    Shutdown();

    return 0;
}
