# RRS — комплект для запуска

Готовая сборка RRS: бинарники, библиотеки, плагины и данные. Запускать
приложения нужно из каталога `bin`: корень комплекта они вычисляют как каталог
на уровень выше текущего рабочего, а не по пути к исполняемому файлу.

## 1. Зависимости системы

Комплект содержит только артефакты сборки: бинарники, библиотеки RRS и
зависимостей (`lib/`), плагины (`plugins/`), данные (`cfg/`, `data/`,
`routes/`), шрифты и темы (`fonts/`, `themes/`). Системных библиотек в нём нет,
поэтому на машине, где вы будете запускать собранное, нужен минимальный набор
пакетов. Установка одной командой (Ubuntu 26.04 или новее):

```bash
sudo apt update
sudo apt install libqt6core6t64 libqt6gui6 libqt6widgets6 libqt6network6 \
    libqt6xml6 qt6-qpa-plugins libqt6serialbus6 libqt6pdf6 \
    libvulkan1 mesa-vulkan-drivers \
    liblua5.4-0 libfreetype6 libassimp6 libopenal1 \
    libcurl4t64 \
    libsfml-system3.0 libsfml-window3.0 \
    fontconfig fonts-dejavu-core
```

- `-dev`-пакеты не нужны: они требуются только для сборки;
- набор рассчитан на docker-сборку (Ubuntu 26.04). Нативная сборка привязывается
  к системе сборки: собранная на Ubuntu 26.10 требует Qt 6.11 и SFML 3.1
  (`libsfml-system3.1`, `libsfml-window3.1`), которых в 26.04 нет, — такой
  комплект запустится только на 26.10 и новее;
- на 24.04 и старше комплект не запустится;
- `LD_LIBRARY_PATH` задавать не нужно: у бинарников прописан RPATH на `../lib`;
- без видеокарты `mesa-vulkan-drivers` даёт программный Vulkan (lavapipe) —
  запускать с `LIBGL_ALWAYS_SOFTWARE=1`.

Проверить, что на машине есть всё нужное (выводятся только ненайденные
библиотеки; `modules/` указан в `LD_LIBRARY_PATH` потому, что `ldd` не смотрит
в каталог самого файла):

```bash
cd bin
for f in * ../lib/*.so* ../modules/*.so ../plugins/*.so; do
    LD_LIBRARY_PATH="$PWD/../lib:$PWD/../modules" ldd "$f" 2>/dev/null
done | awk '/=> not found/{print $1}' | sort -u
```

Пустой вывод — всё найдено. Если что-то вывелось, пакет подбирается по имени
библиотеки: `libQt6SerialBus.so.6` → `libqt6serialbus6`, `libQt6Pdf.so.6` →
`libqt6pdf6`, `libcurl.so.4` → `libcurl4t64`,
`libsfml-system.so.3.0` → `libsfml-system3.0`,
`libsfml-window.so.3.0` → `libsfml-window3.0`,
`liblua5.4.so.0` → `liblua5.4-0`.

Появились `libsfml-*.so.3.1` — это нативная сборка на Ubuntu 26.10; для 26.04
она не предназначена. Строка вида `version 'Qt_6.11' not found` — тоже не
отсутствующий пакет, а слишком старый Qt в системе: комплект собран с более
новым. И то, и другое лечится обновлением системы или использованием
docker-сборки.

## 2. Запуск

```bash
cd bin
./launcher
```

- `launcher` — выбор маршрута и поезда, запуск симуляции;
- `simulator` — запуск симуляции без этого меню:
  `./simulator --route=experimental-polygon_v2.0 --scenario=hello_scenario`
  (имя маршрута — имя каталога в `routes/`, сценарий — имя каталога в
  `scenarios/` внутри маршрута; у маршрута `experimental-polygon` каталога
  `scenarios/` нет);
- `viewer` — просмотр маршрута. Вьювер подключается к серверу на
  `127.0.0.1:1992`, поэтому сервер нужно запустить первым:
  ```bash
  ./server-daemon --port 1992    # в одном терминале
  ./viewer                       # в другом
  ```
  Порт `1992` указан явно, потому что в `cfg/daemon-config.xml` у сервера
  по умолчанию `1982`, а вьювер по умолчанию подключается к `1992`;
- `route_editor` — редактор маршрута.

Снимок экрана вьювера — клавиша **F12**, файлы складываются в `screenshots/`
(каталог создаётся скриптом сборки; без него вьювер пишет
«Fail to write screenchot»).

Маршруты лежат в `routes/`. Их копирует скрипт сборки (`--routes all` или
список имён; при `--routes none` каталог остаётся пустым). Лаунчер просматривает
только вложенные каталоги `routes/`, а название маршрута берёт из
`description.xml` (тег `Route/Title`).