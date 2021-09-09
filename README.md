bhktools
=======
Benualdo's HumanKind Tools

- bhkmap: displays a HumanKind map .hmap file with various useful informations like resources.

# Build
Open bhktools.sln using Visual Studio 2017 or later then build solution :)

All dependencies but the SFML have been copied to this repo, you will need to set an environment variable named $(SFML) pointing to your SFML install in order to compile.

i.e.
SFML=G:\SFML\SFML-2.5.1

# Changelog

v0.41
-----
* Fix corrupted zip file export

v0.4
-----
* Export player spawn locations
* Randomize player spawn order

![ScreenShot040a](doc/img/screen040a.png?raw=true "Screenshot040a")

![ScreenShot040b](doc/img/screen040b.png?raw=true "Screenshot040b")

v0.33
-----
* Export "UseMapCycling"
* Export "UseProceduralMountainChains"

![ScreenShot033](doc/img/screen033.png?raw=true "Screenshot033")

v0.32
-----
* Display player spawns
* Improve heightfield and territory shader to show continent and island shapes better
* Increase max shader read buffer size from 4k to 16k to handle bigger files

![ScreenShot032](doc/img/screen032.png?raw=true "Screenshot032")

v0.31
-----
* Display tile type
* Resource counters
* Hide unselected natural wonders
* Fix crash with ImGui::TreePop when some controls were collapsed

![ScreenShot031](doc/img/screen031.png?raw=true "Screenshot031")

v0.3
-----
* Fix resource draw order
* Clean unused menus

v0.24
-----
* Replicate hexes distortion on textures and icons

![ScreenShot024](doc/img/screen024.png?raw=true "Screenshot024")

v0.23
-----
* Refactor menus
* Display natural wonders as ressources with custom colors 

![ScreenShot023](doc/img/screen023.png?raw=true "Screenshot023")

v0.22
-----
* Refactor shaders (per-texel vs. global flags)
* Refactor map save
* Display natural wonders

![ScreenShot022](doc/img/screen022.png?raw=true "Screenshot022")

v0.21
-----
* Save .hmap files
* Remove landmarks at save to fix corrupted save files

![ScreenShot021](doc/img/screen021.png?raw=true "Screenshot021")

v0.20
-----
* Display resource icons
* Change mouse pan to left button to match the in-game editor
* Display territory biomes
* Improve shaders

![ScreenShot020](doc/img/screen020.png?raw=true "Screenshot020")

v0.13
-----
* Add shader manager
* Display territory borders

![ScreenShot013](doc/img/screen013.png?raw=true "Screenshot013")

v0.12
-----
* Pan & Zoom using SFML views
* Improve the "Debug" window and add an "Help" window
* Switch to light theme

![ScreenShot012](doc/img/screen012.png?raw=true "Screenshot012")

v0.11
-----
* Add camera pan using left mouse button
* Add menu for tool windows
* Fix docking issue with tool windows

v0.1
-----
* Initial commit
* Open .hmap file and decode data

![ScreenShot0](doc/img/screen.png?raw=true "Screenshot0")

Dependencies
-----

* [SFML](https://github.com/SFML/SFML) >= 2.5.0
* [Dear ImGui](https://github.com/ocornut/imgui) >= 1.80
* [ImGui-SFML](https://github.com/eliasdaler/imgui-sfml) >= 2.3+
* [ImGui-Addons](https://github.com/wflohry/imgui-addons)
* [miniz-cpp](https://github.com/tfussell/miniz-cpp)
* [TinyXML-2](https://github.com/leethomason/tinyxml2)
* [Base64](https://github.com/terrakuh/base64)
* [stb](https://github.com/nothings/stb)
