# Сборка RRS на macOS

Проверено: macOS 26.3, Apple Silicon (arm64), AppleClang 17.

## Зависимости

### Homebrew

```bash
brew install cmake qt@6 openal-soft sfml molten-vk \
             vulkan-headers vulkan-loader glslang assimp freetype
```

### Сборка из исходников

```bash
DEPS=$HOME/RRS-deps
PREFIX=$DEPS/install
mkdir -p $DEPS && cd $DEPS
```

**VulkanSceneGraph:**
```bash
git clone https://github.com/vsg-dev/VulkanSceneGraph.git
cd VulkanSceneGraph
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PREFIX
cmake --build build -j$(sysctl -n hw.ncpu) && cmake --install build
cd $DEPS
```

**vsgXchange:**
```bash
git clone https://github.com/vsg-dev/vsgXchange.git
cd vsgXchange
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_PREFIX_PATH=$PREFIX
cmake --build build -j$(sysctl -n hw.ncpu) && cmake --install build
cd $DEPS
```

**vsgImGui:**
```bash
git clone https://github.com/maisvendoo/vsgImGui.git
cd vsgImGui
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_PREFIX_PATH=$PREFIX
cmake --build build -j$(sysctl -n hw.ncpu) && cmake --install build
cd $DEPS
```

**Lua 5.4:**
```bash
git clone https://github.com/maisvendoo/lua-cmake.git
cd lua-cmake
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PREFIX
cmake --build build -j$(sysctl -n hw.ncpu) && cmake --install build
cd $DEPS
```

**sol2:**
```bash
git clone https://github.com/maisvendoo/sol2.git
cd sol2
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_PREFIX_PATH=$PREFIX
cmake --build build -j$(sysctl -n hw.ncpu) && cmake --install build
```

## Сборка RRS

```bash
cd /path/to/RRS
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$HOME/RRS-deps/install;/opt/homebrew/opt/openal-soft;/opt/homebrew"
cmake --build build -j$(sysctl -n hw.ncpu)
```

Результат: исполняемые файлы в `bin/`, библиотеки в `lib/`.

## Запуск

```bash
cd bin/

# Терминал 1 — симулятор
./simulator --route experimental-polygon_v2.0 --scenario hello_scenario

# Терминал 2 — вьювер
VK_ICD_FILENAMES=$(brew --prefix molten-vk)/etc/vulkan/icd.d/MoltenVK_icd.json \
QT_QPA_PLATFORM=offscreen \
./viewer
```

`VK_ICD_FILENAMES` — путь к ICD MoltenVK (keg-only в Homebrew).
`QT_QPA_PLATFORM=offscreen` — обход конфликта NSApp между Qt и VSG.
