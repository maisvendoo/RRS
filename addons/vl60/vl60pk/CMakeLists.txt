# Минимальная требуемая версия CMake
cmake_minimum_required (VERSION 3.7)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET vl60pk)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../../modules/${TARGET}")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_MODULE_PREFIX "")

# Ищем зависимости Qt5
find_package(Qt5 COMPONENTS Xml REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB EQP_INCLUDES "../vl60-equipment/include/*.h")
file (GLOB SOURCES "./src/*.cpp")
file (GLOB EQP_SOURCES "../vl60-equipment/src/*.cpp")

# Компиляция библиотеке
add_library (${TARGET} MODULE ${SOURCES} ${INCLUDES} ${EQP_SOURCES} ${EQP_INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../vl60-equipment/include/)
target_include_directories (${TARGET} PRIVATE ../../../CfgReader/include)
target_include_directories (${TARGET} PRIVATE ../../../common-headers)
target_include_directories (${TARGET} PRIVATE ../../../simulator/solver/include)
target_include_directories (${TARGET} PRIVATE ../../../simulator/physics/include)
target_include_directories (${TARGET} PRIVATE ../../../simulator/vehicle/include)
target_include_directories (${TARGET} PRIVATE ../../../simulator/device/include)
target_include_directories (${TARGET} PRIVATE ../../../filesystem/include)

# Компонуемся с библиотеками Qt5
target_link_libraries (${TARGET} Qt5::Xml)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} physics)
target_link_libraries (${TARGET} vehicle)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} filesystem)
