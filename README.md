# Russian Railway Simulator (RRS)

This project is a free, open-source railway simulator of Russian mainline locomotives and trains. The game features an original train physics engine that accounts for longitudinal dynamics, wheel–rail interaction, and the operation of pneumatic and electrical systems on both locomotives and railcars. Players can take on roles such as locomotive engineer, assistant engineer, dispatcher, or rolling stock maintenance personnel.

![](https://habrastorage.org/webt/nj/zq/gs/njzqgs4bdi8i73r5emhjm7b-ock.jpeg)

The game's graphics are built upon the latest [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph) rendering library and utilize the open [glTF 2.0](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html) model format with support for PBR (Physically Based Rendering) materials.

Main languge for this game development is C++17. You must ensure that your C++ compiler supports this language standard. It is expected that for building on unix-like operating systems, the GCC compiler will be used, and for Windows builds, the MinGW compiler version 13.1.0 or higher will be used.

The game is distributed together with an SDK containing header files, libraries, and CMake configurations for developing custom locomotive, railcar, and equipment modules. You can find it in sdk/ directory of game.

Dependencies:

* [Qt6](https://www.qt.io/development/qt-framework/qt6) - crossplatform framework for C++ development
* [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph), [vsgXchange](https://github.com/vsg-dev/vsgXchange), [vsgImGui](https://github.com/maisvendoo/vsgImGui) – components for graphics rendering, model handling, and user interface creation
* [glslang](https://github.com/KhronosGroup/glslang) – shader compiler
* [OpenAL](https://github.com/kcat/openal-soft) – audio library
* [FreeType2](https://gitlab.freedesktop.org/freetype/freetype) – font rendering library
* [assimp](https://github.com/assimp/assimp) – library for loading 3D models and images
* [SFML](https://github.com/SFML/SFML) – used for interfacing with joysticks and controls that simulate locomotive cab instruments
* [Lua](https://github.com/maisvendoo/lua-cmake) – scripting language for writing game scenarios
* [sol2](https://github.com/maisvendoo/sol2) – C++ binding library for Lua integration
* [KTX-Software](https://github.com/khronosgroup/ktx-software) - libraries and soft for a work with Khronos Textures (KTX)

For more information, you can visit our [official website](https://rusrailsim.ru) as well as our group on the [VK social network](https://vk.com/rusrailsim) (both in Russian). They also contain a [detailed guide for building the game on Windows](https://www.rusrailsim.ru/support/developer_guide/#windows_10/11_64-bit) and include add-ons that extend the game's functionality.

## Add-ons

The following add-on modules extend the game with additional locomotive and rolling stock models. Each repository contains the complete source code, configuration files, and 3D models for the respective vehicle, and can be built using the RRS SDK.

### Locomotives

| Vehicle | Repository | Description |
|---------|-----------|-------------|
| **TEP70** | [maisvendoo/tep70](https://github.com/maisvendoo/tep70) | Mainline diesel locomotive with AC/AC power transmission, 3000 hp |
| **EP1M** | [maisvendoo/ep1m](https://github.com/maisvendoo/ep1m) | AC mainline electric locomotive, 8200 kW (twin-unit) |
| **ChS2T** | [RRS-dev/chs2t](https://codeberg.org/RRS-dev/chs2t) | DC mainline electric locomotive, 4590 kW |
| **TEM15** | [RRS-dev/tem15](https://codeberg.org/RRS-dev/tem15) | Shunting diesel locomotive with hydraulic transmission, 1350 hp |

### Railcars

| Vehicle | Repository | Description |
|---------|-----------|-------------|
| **RA3** | [codeberg.org/RRS-dev/ra3](https://codeberg.org/RRS-dev/ra3) | Diesel rail bus for suburban passenger service |
| **Cisterns Pack** | [codeberg.org/RRS-dev/cisterns_pack](https://codeberg.org/RRS-dev/cisterns_pack) | Set of tank car variants for liquid cargo transportation |
