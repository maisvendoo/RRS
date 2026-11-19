# Сборка RRS под Linux (Docker и нативная, Убунту 26.04)

Два способа получить рабочую сборку:

1. **В Docker-контейнере** на базе **Убунту 26.04 LTS** — контейнер нужен только
   для сборки, готовые файлы забираются в `dist/`, после чего контейнер можно
   удалить. Сборка образа описана в файле
   [`../Dockerfile.ubuntu26.04`](../Dockerfile.ubuntu26.04), управление сборкой —
   скрипт [`../scripts/build_docker.sh`](../scripts/build_docker.sh).
2. **Нативно**, без Docker — системные пакеты ставятся в Убунту, зависимости из
   исходников собираются в локальный префикс. Скрипт
   [`../scripts/build_linux.sh`](../scripts/build_linux.sh), подробности в
   разделе «[Сборка без Docker](#сборка-без-docker-убунту-2604-и-новее)».

## Что нужно (для сборки в Docker)

- Убунту (или другой Linux) с установленным и запущенным Docker;
- `git` — для получения исходников;
- доступ в интернет: `apt`, `git clone` (vsg, vsgImGui, KTX);
- примерно 25 ГБ свободного места (образ + кеш сборки);
- место под результат: около 550 МБ на `dist/` без маршрутов и до 1,3 ГБ —
  со всеми четырьмя тестовыми маршрутами;
- 64-разрядная ОС (артефакты собираются под `x86_64`).

## Быстрый старт

```bash
cd <путь к репозиторию>/RRS
./scripts/build_docker.sh
```

Скрипт сам соберёт образ и выложит результат в `dist/`:

```
dist/
  bin/        исполняемые файлы (launcher, simulator, viewer, утилиты)
  lib/        библиотеки vsg, vsgXchange, vsgImGui, ktx
  modules/    модели подвижного состава и приборов, lua/ — модули сценариев
  plugins/    плагины
  sdk/        заголовки SDK для сторонних модулей
  cfg/, data/ конфигурация и данные (нужны для запуска)
  routes/     маршруты (копируются из каталога routes/ репозитория)
```

Маршруты (около 750 МБ на все четыре тестовых полигона) в образ не входят: они
не нужны на этапе компиляции, а их исключение из контекста сборки экономит время
и трафик. Скрипт копирует их из `routes/` в `dist/routes/` после выгрузки
артефактов — уже готовыми к запуску. К размеру `dist/` прибавьте размер
скопированных маршрутов.

Полезные варианты запуска:

| Команда | Что делает |
|---|---|
| `./scripts/build_docker.sh --build-only` | только собрать образ, без выгрузки |
| `./scripts/build_docker.sh --no-build` | только выгрузить артефакты из готового образа |
| `./scripts/build_docker.sh --deps-only` | собрать образ только с зависимостями |
| `./scripts/build_docker.sh -j 8 -o dist` | 8 задач сборки, результат в `dist` |
| `./scripts/build_docker.sh -t Debug` | отладочная сборка |
| `./scripts/build_docker.sh --no-cache` | без кеша (нужно при смене версий зависимостей) |
| `./scripts/build_docker.sh --routes experimental-polygon_v2.0_ktx` | положить в `dist/routes/` только один маршрут |
| `./scripts/build_docker.sh --routes none` | не копировать маршруты вообще |
| `RRS_ROUTES=... ./scripts/build_docker.sh` | то же через переменные `RRS_ROUTES` / `RRS_ROUTES_SRC` |

Те же `--routes` / `--no-routes` понимает и `../scripts/build_linux.sh` — сборка
без Docker, с установкой в каталог: `./scripts/build_linux.sh --install dist --routes none`.

## Ручная сборка в Docker (без скрипта)

```bash
docker build -f Dockerfile.ubuntu26.04 -t rrs-build:ubuntu-26.04 .

# забрать результат из образа
CID=$(docker create rrs-build:ubuntu-26.04)
docker cp "$CID:/opt/rrs-dist/." dist/
docker rm "$CID"

# маршруты в образ не входят — скопировать их отдельно
cp -a routes/. dist/routes/
```

## Сборка без Docker (Убунту 26.04 и новее)

Системные библиотеки ставятся пакетами дистрибутива, а то, чего в репозиториях
Убунту нет — **VulkanSceneGraph, vsgXchange, vsgImGui и KTX** — собирается из
исходников в локальный префикс `.deps/install`. Все шаги выполняет скрипт
[`../scripts/build_linux.sh`](../scripts/build_linux.sh); ниже описано, что именно
он делает и как повторить это вручную.

### 1. Системные пакеты

```bash
sudo apt install build-essential ninja-build pkg-config git cmake \
    qt6-base-dev qt6-pdf-dev qt6-serialbus-dev qt6-l10n-tools qt6-tools-dev \
    libvulkan-dev vulkan-tools glslang-tools glslang-dev \
    libgl1-mesa-dev libglu1-mesa-dev \
    libopenal-dev liblua5.4-dev \
    libassimp-dev libfreetype-dev zlib1g-dev libpng-dev \
    libx11-dev libxcb1-dev libxkbcommon-x11-dev libwayland-dev \
    libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
    libsfml-dev sol2-dev
```

| Пакеты | Что дают |
|---|---|
| `build-essential`, `git`, `pkg-config` | компилятор, доступ к исходникам, поиск библиотек |
| `cmake` (≥ 3.29), `ninja-build` | сама сборка |
| `qt6-base-dev`, `qt6-tools-dev` | GUI; `qt6-tools-dev` даёт `Qt6LinguistTools` — без него не собирается лаунчер |
| `qt6-pdf-dev`, `qt6-serialbus-dev`, `qt6-l10n-tools` | просмотр PDF, шина CAN, утилиты переводов |
| `libvulkan-dev`, `vulkan-tools`, `glslang-tools`, `glslang-dev` | Vulkan, `vulkaninfo` и компиляция GLSL в SPIR-V; без glslang вьювер падает с `not compiled with GLSLang` |
| `libgl1-mesa-dev`, `libglu1-mesa-dev` | OpenGL и GLU для vsg |
| `libopenal-dev` | звук |
| `liblua5.4-dev` | сценарии на Lua (`modules/lua`) |
| `libassimp-dev`, `libfreetype-dev`, `zlib1g-dev`, `libpng-dev` | загрузка моделей, шрифты, сжатие текстур |
| `libsfml-dev`, `sol2-dev` | SFML 3.0 для симулятора, Lua-биндинги |
| `libx11-dev`, `libxcb1-dev`, `libxkbcommon-x11-dev`, `libwayland-dev`, `libxrandr-dev`, `libxcursor-dev`, `libxi-dev`, `libudev-dev` | окна и ввод Qt |

Проверить, что всё нашлось (скрипт сверяет инструменты, CMake-конфиги и
`pkg-config`-модули):

```bash
./scripts/build_linux.sh --check-only
```

### 2. Сборка скриптом

```bash
./scripts/build_linux.sh -j "$(nproc)"                       # зависимости + RRS
./scripts/build_linux.sh -j "$(nproc)" --install "$PWD/dist" # то же + полный комплект
```

Что делает скрипт:

1. проверяет системные зависимости (см. `--check-only`);
2. собирает из исходников то, чего нет в пакетах, в `.deps/install`
   (исходники — в `.deps/src`): `vsg` v1.1.16, `vsgXchange` v1.1.14,
   `vsgImGui` (ветка `master-RRS`), `KTX` (коммит `983695c105618462b8e96458b378222e99faec98`),
   а `sol2` и `SFML` — только если их системных CMake-конфигураций не нашлось;
3. конфигурирует и собирает RRS в `build/` (CMake + Ninja);
4. с `--install` выполняет `cmake --install`, копирует артефакты сборки
   (`bin`, `lib`, `modules`, `plugins`, `cfg`, `data`) и маршруты в каталог
   установки.

Уже собранные зависимости повторно не собираются — повторный запуск продолжает
с места остановки.

Артефакты native-сборки кладутся **не** в `build/`: проект задаёт выходные пути
относительно каталога сборки (`../../bin`, `../../lib`), поэтому исполняемые
файлы, библиотеки, модули и плагины оказываются в корне репозитория —
в `bin/`, `lib/`, `modules/`, `plugins/`. Там же лежат `cfg/` и `data/`, а
маршруты — в `routes/`, так что комплект для запуска собирать не нужно.

Сам `cmake --install` кладёт только `sdk/` (CMake-пакет для сторонних модулей)
и `modules/lua/`, поэтому `--install` дополнительно копирует `bin/`, `lib/`,
`modules/`, `plugins/`, `cfg/`, `data/` и библиотеки зависимостей из
`.deps/install/lib` — иначе в каталоге установки не было бы ни одного
исполняемого файла. Маршруты попадают в `routes/` рядом.

Основные параметры: `--deps-only`, `--skip-deps`, `--rebuild-deps`, `--clean`,
`clean` (полная очистка, см. ниже), `-t Debug`, `-b <каталог сборки>`,
`--deps-prefix <каталог>`, `--generator <генератор>`,
`--routes СПИСОК` / `--no-routes`, `--check-only`.

### 2.1 Полная очистка (цель `clean`)

```bash
./scripts/build_linux.sh clean
```

Удаляет всё, что появилось в результате сборки и установки:

| Что удаляется | Путь |
|---|---|
| Каталог сборки CMake | `build/` (или `--build-dir`, `RRS_BUILD_DIR`) |
| Исполняемые файлы | `bin/` |
| Библиотеки | `lib/` |
| Подключаемые модули | `modules/` |
| Плагины | `plugins/` |
| Каталог установки | `dist/` (или путь из `--install`) |
| Зависимости: исходники и сборки | `.deps/src/` |
| Зависимости: установленные | `.deps/install/` |
| Каталог зависимостей целиком | `.deps/` |

`cfg/` и `data/` не трогаются: они входят в репозиторий, это исходные данные, а
не результат сборки.

Свои пути (если каталоги переопределялись при сборке):

```bash
./scripts/build_linux.sh clean --install /opt/rrs -b build-debug --deps-prefix /opt/rrs-deps
```

Особенности:

- `clean` ничего не собирает и не проверяет зависимости — его можно вызвать на
  сломанном дереве;
- повторный вызов безопасен: уже удалённое печатается как `[НЕТ]`;
- есть защита от удаления `/` и корня репозитория;
- после `clean` зависимости (VulkanSceneGraph, vsgXchange, vsgImGui, KTX)
  собираются заново — это 10–20 минут; если пересборка не нужна, собирайте
  зависимости отдельно: `./scripts/build_linux.sh --deps-only`.

Не путать с параметром `--clean`: тот удаляет только каталог сборки перед
конфигурированием при обычном запуске.

### 3. Ручная сборка (только cmake)

Если скрипт по каким-то причинам не подходит, те же шаги командами:

```bash
DEPS="$PWD/.deps/install"

# VulkanSceneGraph: ShaderCompiler обязателен, ShaderOptimizer выключен
git clone --depth 1 --branch v1.1.16 \
    https://github.com/vsg-dev/VulkanSceneGraph.git .deps/src/vsg
cmake -S .deps/src/vsg -B .deps/src/vsg/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_INSTALL_PREFIX="$DEPS" -DCMAKE_PREFIX_PATH="$DEPS" \
      -DVSG_SUPPORTS_ShaderCompiler=ON -DVSG_SUPPORTS_ShaderOptimizer=OFF \
      -DVSG_SUPPORTS_Windowing=ON
cmake --build .deps/src/vsg/build -j "$(nproc)"
cmake --install .deps/src/vsg/build

# vsgXchange
git clone --depth 1 --branch v1.1.14 \
    https://github.com/vsg-dev/vsgXchange.git .deps/src/vsgXchange
cmake -S .deps/src/vsgXchange -B .deps/src/vsgXchange/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_INSTALL_PREFIX="$DEPS" -DCMAKE_PREFIX_PATH="$DEPS"
cmake --build .deps/src/vsgXchange/build -j "$(nproc)"
cmake --install .deps/src/vsgXchange/build

# vsgImGui: ветка master-RRS (в vsgImGui 0.9 и стабильных тегах vsg 1.1.16
# использует поля vsg::GraphicsPipelineState, удалённые в vsg)
git clone --depth 1 --recursive --shallow-submodules --branch master-RRS \
    https://github.com/maisvendoo/vsgImGui.git .deps/src/vsgImGui
cmake -S .deps/src/vsgImGui -B .deps/src/vsgImGui/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_INSTALL_PREFIX="$DEPS" -DCMAKE_PREFIX_PATH="$DEPS"
cmake --build .deps/src/vsgImGui/build -j "$(nproc)"
cmake --install .deps/src/vsgImGui/build

# KTX-Software: закреплённый коммит (в релизах 4.x vsgXchange не находит
# нужные ktxTexture_* функции). --branch с SHA-1 git не принимает, поэтому
# сначала обычное клонирование, затем fetch и checkout коммита
git clone https://github.com/KhronosGroup/KTX-Software.git .deps/src/KTX-Software
git -C .deps/src/KTX-Software fetch --depth 1 origin 983695c105618462b8e96458b378222e99faec98
git -C .deps/src/KTX-Software checkout FETCH_HEAD
cmake -S .deps/src/KTX-Software -B .deps/src/KTX-Software/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_INSTALL_PREFIX="$DEPS" -DCMAKE_PREFIX_PATH="$DEPS" \
      -DKTX_FEATURE_TESTS=OFF -DKTX_FEATURE_TOOLS=OFF -DKTX_FEATURE_DOC=OFF \
      -DKTX_FEATURE_JNI=OFF -DKTX_FEATURE_PY=OFF \
      -DKTX_FEATURE_VK_UPLOAD=ON -DKTX_FEATURE_GL_UPLOAD=OFF
cmake --build .deps/src/KTX-Software/build -j "$(nproc)"
cmake --install .deps/src/KTX-Software/build

# сам RRS
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$PWD/.deps/install" \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
      -DCLIENT_BUILD=ON -DBUILD_TESTING=OFF
cmake --build build -j "$(nproc)"
```

### 4. Запуск после нативной сборки

```bash
cd bin
./launcher
```

- запускать нужно из `bin/` в корне репозитория: корень комплекта приложения
  вычисляют как каталог на уровень выше рабочего (см. «Запуск собранного»);
- маршруты берутся из `routes/` в корне репозитория;
- шрифты и темы оформления берутся из `fonts/` и `themes/` корня комплекта: при
  запуске из `dist/bin` эти каталоги должны лежать в `dist/` (скрипт копирует их
  при `--install`), иначе вьювер пишет `Could not load font file!` и падает;
- нужен Vulkan-драйвер. Для проверки без видеокарты:
  `sudo apt install mesa-vulkan-drivers` и `LIBGL_ALWAYS_SOFTWARE=1`;
- симулятор запускается так же: `./simulator --route=<имя маршрута> --scenario=hello_scenario`
  (имя маршрута — имя каталога в `routes/`, сценарий — имя каталога в `scenarios/`).

### 5. Отличия нативной сборки от Docker-сборки

- **CMake 4.x** (в Убунту 26.04) несовместим с `cmake_minimum_required < 3.5`
  в конфигурациях части зависимостей. Скрипт добавляет
  `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`; при ручном конфигурировании добавляйте
  этот флаг, если видите ошибку про `cmake_minimum_required`.
- **`glslang`** — пакеты `glslang-tools` и `glslang-dev` (пакета `libglslang-dev`
  в Убунту 26.04 нет). `glslang-dev` обязателен: без его CMake-конфигурации
  VulkanSceneGraph молча выключает компиляцию шейдеров (`VSG_SUPPORTS_ShaderCompiler`),
  и симулятор с вьювером падают с `not compiled with GLSLang`.
- **`SFML 3.0` и `sol2 3.5`** берутся пакетами; из исходников скрипт собирает их
  только если системных конфигураций нет.
- **Версии** зависимостей по умолчанию совпадают с Dockerfile и переопределяются
  переменными окружения: `VSG_VERSION`, `VSGXCHANGE_VERSION`, `VSGIMGUI_VERSION`,
  `SOL2_VERSION`, `KTX_VERSION`.
- **Совместимость с системой**: нативная сборка работает только на Убунту 26.04
  и новее (glibc 2.42); Docker-сборка переносима между хостами.

## Что происходит внутри образа

1. **Стадия `deps`** — системные пакеты Убунту 26.04 (CMake 4.2, Ninja, Qt 6.10,
   Vulkan, glslang, OpenAL, Lua 5.4, assimp, FreeType, SFML 3.0, sol2, X11/xcb)
   и исходники, собранные вручную в `/opt/rrs-deps`:
   `vsg` v1.1.16, `vsgXchange` v1.1.14, `vsgImGui` (ветка `master-RRS`),
   `KTX-Software` (конкретный коммит `main`, см. ниже).
2. **Стадия `RRS`** — сборка самого симулятора в `/opt/rrs-dist`.

Слои кешируются: при изменении только кода RRS пересобирается последняя стадия.

Версии зависимостей задаются через `ARG` в Dockerfile или переменные окружения
(`VSG_VERSION`, `VSGXCHANGE_VERSION`, `VSGIMGUI_VERSION`, `KTX_VERSION`).
При их изменении собирайте с `--no-cache`.

## Особенности этой сборки

- **`glslang`** ставится пакетом `glslang-dev` (в Убунту 26.04 он так называется,
  пакета `libglslang-dev` больше нет). Без него VulkanSceneGraph молча выключает
  ShaderCompiler, и вьювер падает с `not compiled with GLSLang`.
- **vsg ShaderCompiler** собирается включённым (`-DVSG_SUPPORTS_ShaderCompiler=ON`),
  ShaderOptimizer — выключенным (нет `SPIRV-Tools-opt`).
- **KTX-Software** собирается из конкретного коммита `main`: в релизах 4.x нет
  поля `ktxBasisParams::codec`, которое использует код RRS.
- **`SFML 3.0`, `sol2 3.5`, `Qt 6.10`** берутся из репозиториев Убунту 26.04,
  из исходников не собираются.

## Запуск собранного

Приложения RRS вычисляют корень комплекта от **текущего рабочего каталога**
(корень = каталог на уровень выше рабочего), а не от пути к исполняемому файлу.
Поэтому запускать нужно из каталога `bin/`:

```bash
cd dist/bin
./launcher
```

- `cfg/`, `data/`, `routes/`, `modules/`, `plugins/`, `lib/` должны лежать на
  уровень выше `bin/`, то есть в `dist/`;
- если запустить `./dist/bin/launcher` из корня комплекта, лаунчер откроется, но
  список маршрутов будет пустым — маршруты он ищет в каталоге на уровень выше
  рабочего;
- нужен X11 (или Wayland) и Vulkan-драйвер; для проверки без видеокарты —
  `LIBGL_ALWAYS_SOFTWARE=1` и `QT_QPA_PLATFORM=xcb` (Mesa/lavapipe);
- маршруты лежат в `dist/routes/` — их копирует туда скрипт сборки
  (см. `--routes`); лаунчер просматривает только вложенные каталоги, а название
  маршрута берёт из `description.xml` (тег `Route/Title`).

Какие пакеты нужны в системе для такого запуска — в разделе «Что установить в
систему для запуска Docker-сборки» ниже.

## Что установить в систему для запуска Docker-сборки

В `dist/` лежат только артефакты сборки: бинарники, библиотеки RRS (`lib/`),
плагины (`plugins/`) и данные (`cfg/`, `data/`, `routes/`). Системных библиотек там
нет, поэтому на машине, где вы будете запускать собранное, нужен минимальный набор
пакетов. Для Убунту 26.04 он такой:

```bash
sudo apt install libqt6core6t64 libqt6gui6 libqt6widgets6 libqt6network6 \
    libqt6xml6 qt6-qpa-plugins libqt6serialbus6 libqt6pdf6 \
    libvulkan1 mesa-vulkan-drivers \
    liblua5.4-0 libfreetype6 libassimp6 libopenal1 \
    libsfml-system3.0 libsfml-window3.0 \
    fontconfig fonts-dejavu-core
```

| Пакеты | Что дают |
|---|---|
| `libqt6core6t64`, `libqt6gui6`, `libqt6widgets6`, `libqt6network6`, `libqt6xml6` | Qt6: ядро, GUI, виджеты, сеть, XML |
| `qt6-qpa-plugins` | плагин платформы xcb и его зависимости (`libxcb-cursor0`, `libxkbcommon-x11-0` и т. д.) |
| `libqt6serialbus6`, `libqt6pdf6` | шина CAN, просмотр PDF |
| `libvulkan1`, `mesa-vulkan-drivers` | загрузчик Vulkan и программный драйвер lavapipe |
| `liblua5.4-0` | сценарии на Lua |
| `libfreetype6`, `libassimp6`, `libopenal1` | шрифты, модели, звук |
| `libsfml-system3.0`, `libsfml-window3.0` | SFML 3.0 для симулятора |
| `fontconfig`, `fonts-dejavu-core` | поиск шрифтов и сами шрифты |

- `-dev`-пакеты не нужны — они требуются только для сборки (см. «Системные пакеты»);
- система должна быть **Убунту 26.04 или новее**: артефакты собраны на 26.04
  (glibc 2.42), на 24.04 и старше они не запустятся — используйте контейнер
  `ubuntu:26.04` с примонтированным `dist/`;
- `LD_LIBRARY_PATH` при запуске задавать не нужно: у бинарников прописан RPATH
  на `../lib`;
- без видеокарты `mesa-vulkan-drivers` даёт программный Vulkan (lavapipe), а без
  дисплея помогает `xvfb` вместе с `QT_QPA_PLATFORM=xcb` и `LIBGL_ALWAYS_SOFTWARE=1`.

Проверить, что на машине есть всё нужное (выводятся только ненайденные библиотеки;
`modules/` указан в `LD_LIBRARY_PATH` потому, что `ldd` не смотрит в каталог самого
файла):

```bash
cd dist
for f in bin/* lib/*.so* modules/*.so plugins/*.so; do
    LD_LIBRARY_PATH="$PWD/lib:$PWD/modules" ldd "$f" 2>/dev/null
done | awk '/not found/{print $1}' | sort -u
```

Пустой вывод — всё найдено. Если что-то вывелось, пакет подбирается по имени
библиотеки: `libQt6SerialBus.so.6` → `libqt6serialbus6`, `libQt6Pdf.so.6` →
`libqt6pdf6`, `libsfml-system.so.3.0` → `libsfml-system3.0`,
`libsfml-window.so.3.0` → `libsfml-window3.0`, `liblua5.4.so.0` →
`liblua5.4-0`.

Проверено: в чистом контейнере `ubuntu:26.04` только с этими пакетами (без
компилятора и `-dev`-пакетов) проверка выше не находит пропущенных библиотек,
`simulator` работает, `launcher` рисует интерфейс.

## Если что-то не работает

| Симптом | Причина / решение |
|---|---|
| `not compiled with GLSLang` при запуске вьювера | VulkanSceneGraph собран без компилятора шейдеров. Нативная сборка: не установлен `glslang-dev` — поставьте пакет и пересоберите зависимости (`./scripts/build_linux.sh --deps-only`). Docker-образ: пересоберите образ (`--no-cache`) |
| `imgui-error ... Could not load font file!` | в каталоге установки нет `fonts/` (и `themes/`). Скрипт копирует их при `--install`; при ручной сборке скопируйте `fonts/` и `themes/` рядом с `bin/` |
| `zsh: segmentation fault (core dumped) ./viewer` | следствие предыдущих двух пунктов (нет шейдеров и/или шрифта): сначала устраните `not compiled with GLSLang` и `Could not load font file!`, затем запускайте вьювер снова |
| Двоичные файлы не запускаются на хосте | артефакты собраны на Убунту 26.04 (glibc 2.42); на более старом хосте нужен контейнер той же версии |
| `error while loading shared libraries: libQt6*.so.6` | в системе нет пакетов Qt6 — см. «Что установить в систему для запуска Docker-сборки» |
| `Could not load the Qt platform plugin "xcb"` | не установлен `qt6-qpa-plugins` |
| `vsg::Instance::create(...) failed to create VkInstance` | нет Vulkan-драйвера: `sudo apt install mesa-vulkan-drivers` |
| `Не найден .../Dockerfile.ubuntu26.04` | файл сборки переименован или отсутствует; путь можно задать через `RRS_DOCKERFILE` |
| `Образ ... не найден` при `--no-build` | сначала соберите образ: `./scripts/build_docker.sh --build-only` |
| Список маршрутов в лаунчере пуст | лаунчер запущен не из `bin/`; корень комплекта вычисляется от рабочего каталога, см. «Запуск собранного» |
| В `dist/routes/` нет маршрутов | каталог `routes/` пуст или не загружен, либо задан `--routes none`; список задаётся `--routes`/`RRS_ROUTES` |
| `Could not find a package configuration file ... Qt6LinguistTools` (нативная сборка) | не установлен `qt6-tools-dev` — без него не собирается лаунчер |
| `Compatibility with CMake < 3.5 has been removed` (нативная сборка) | старые конфигурации зависимостей; добавьте `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` (скрипт `build_linux.sh` делает это сам) |
| `Could not find vsg / vsgXchange / vsgImGui / KTX` (нативная сборка) | зависимости не собраны: `./scripts/build_linux.sh --deps-only` |
| `module 'triggers_fabric' not found` (нативная сборка) | в сборочном дереве нет `modules/lua` — пересоберите RRS: `cmake --build build --target rrs_lua` |
| Сообщения `proxychains ... DLL init` | посторонний шум перехватчика в окружении, к сборке не относится |

## Смежные документы

- [BUILD_MACOS.md](BUILD_MACOS.md) — сборка на macOS;
- [Руководство разработчика.pdf](Руководство%20разработчика.pdf) — сборка на Windows;
- [README](../README.md) — список зависимостей и общие сведения.