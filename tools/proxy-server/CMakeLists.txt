# Установка целей и библиотек
set (TARGET proxy-server)

# Инициализация проекта
project (${TARGET})

# Qt-автоматическая генерация MOC
set(CMAKE_AUTOMOC ON)

# Каталог размещения исполняемого файла
set (EXECUTABLE_OUTPUT_PATH "../../../bin")

# Поиск необходимых модулей Qt6
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)

# Сбор источников
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Добавление исполняемого файла
add_executable (${TARGET} ${SOURCES} ${INCLUDES})

# Подключение каталога с заголовками
target_include_directories (${TARGET} PRIVATE ./include/)

# Связывание с библиотеками
target_link_libraries (${TARGET} Qt6::Core)
target_link_libraries (${TARGET} Qt6::Network)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} Journal)
