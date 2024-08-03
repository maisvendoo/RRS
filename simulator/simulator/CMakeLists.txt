# Минимальная требуемая версия CMake
cmake_minimum_required (VERSION 3.7)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET simulator)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (EXECUTABLE_OUTPUT_PATH "../../../bin")

# Ищем зависимости Qt5
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Xml REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция приложения
add_executable (${TARGET} ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../physics/include)
target_include_directories (${TARGET} PRIVATE ../solver/include)
target_include_directories (${TARGET} PRIVATE ../vehicle/include)
target_include_directories (${TARGET} PRIVATE ../profile/include)
target_include_directories (${TARGET} PRIVATE ../../CfgReader/include)
target_include_directories (${TARGET} PRIVATE ../../filesystem/include)
target_include_directories (${TARGET} PRIVATE ../../libJournal/include)
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../device/include)
target_include_directories (${TARGET} PRIVATE ../sim-client/include)
target_include_directories (${TARGET} PRIVATE ../../tcp-connection/include)
target_include_directories (${TARGET} PRIVATE ../signaling/include)
target_include_directories (${TARGET} PRIVATE ../train/include)
target_include_directories (${TARGET} PRIVATE ../model/include)

# Компонуемся с библиотеками Qt5
target_link_libraries (${TARGET} Qt5::Core)
target_link_libraries (${TARGET} Qt5::Xml)
target_link_libraries (${TARGET} Qt5::Network)
# и со своими не забываем
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} physics)
target_link_libraries (${TARGET} profile)
target_link_libraries (${TARGET} vehicle)
target_link_libraries (${TARGET} solver)
target_link_libraries (${TARGET} train)
target_link_libraries (${TARGET} tcp-connection)
target_link_libraries (${TARGET} sim-client)
target_link_libraries (${TARGET} signaling)
target_link_libraries (${TARGET} model)
