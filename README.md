bhktools
=======
Benualdo's HumanKind Tools

- bhkmap: displays a HumanKind map .hmap file with various useful informations like resources.

![ScreenShot0](doc/img/screen.png?raw=true "Screenshot0")

# how to build
Open bhktools.sln using Visual Studio 2017 or later then build solution :)

All dependencies but the SFML have been copied to this repo, you will need to set an $(SFML) environment variable named $(SFML) pointing to your SFML install in order to compile.

i.e.
SFML=G:\SFML\SFML-2.5.1

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
