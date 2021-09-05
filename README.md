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
