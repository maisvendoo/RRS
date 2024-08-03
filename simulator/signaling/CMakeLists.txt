# Минимальная требуемая версия CMake
cmake_minimum_required (VERSION 3.7)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET signaling)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Опеределяем дефайн CFG_READER_LIB для корректного
# экспорта и импорта слассов из DLL
add_definitions(-DSIGNALING_LIB)

# Ищем зависимости Qt5
find_package(Qt5 COMPONENTS Xml REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеке
add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../physics/include)
target_include_directories (${TARGET} PRIVATE ../solver/include)
target_include_directories (${TARGET} PRIVATE ../vehicle/include)
target_include_directories (${TARGET} PRIVATE ../../CfgReader/include)
target_include_directories (${TARGET} PRIVATE ../../libJournal/include)
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../device/include)

# Компонуемся с библиотеками Qt5
target_link_libraries (${TARGET} Qt5::Xml)
# и со своими не забываем
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} physics)
