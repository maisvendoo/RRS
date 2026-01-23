cmake_minimum_required(VERSION 3.23)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET topology)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Определяем дефайн TOPOLOGY_LIB для корректного
# экспорта и импорта классов из DLL
add_definitions(-DTOPOLOGY_LIB)

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Xml REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# # Компиляция библиотеки
# add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

add_library(${TARGET} SHARED)

# TODO: Separate private and public headers
target_sources(${TARGET}
    PRIVATE
        ${SOURCES}

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            ${INCLUDES}
)

# Указываем пути поиска заголовков
# target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../libJournal/include)
target_include_directories (${TARGET} PUBLIC ../../common-headers)

# TODO: Could link PRIVATE?
# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} PUBLIC Qt6::Xml)
# и со своими не забываем
target_link_libraries (${TARGET} PUBLIC filesystem)
target_link_libraries (${TARGET} PUBLIC Journal)
target_link_libraries (${TARGET} PUBLIC physics)
target_link_libraries (${TARGET} PUBLIC CfgReader)
target_link_libraries (${TARGET} PUBLIC device)
target_link_libraries (${TARGET} PUBLIC vehicle)
