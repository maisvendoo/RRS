# Russian Railway Simulator (RRS)

This project is a free, open-source railway simulator of Russian mainline locomotives and trains. The game features an original train physics engine that accounts for longitudinal dynamics, wheel–rail interaction, and the operation of pneumatic and electrical systems on both locomotives and railcars. Players can take on roles such as locomotive engineer, assistant engineer, dispatcher, or rolling stock maintenance personnel.

![](https://habrastorage.org/webt/nj/zq/gs/njzqgs4bdi8i73r5emhjm7b-ock.jpeg)

The game's graphics are built upon the latest [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph) rendering library and utilize the open [glTF 2.0](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html) model format with support for PBR (Physically Based Rendering) materials.

Main languge for this game development is C++17. You must ensure that your C++ compiler supports this language standard. It is expected that for building on unix-like operating systems, the GCC compiler will be used, and for Windows builds, the MinGW compiler version 13.1.0 or higher will be used.

The game is distributed together with an SDK containing header files, libraries, and CMake configurations for developing custom locomotive, railcar, and equipment modules. You cant find it in sdk/ directory of game.

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

