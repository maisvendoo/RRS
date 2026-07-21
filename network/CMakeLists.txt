# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET network)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Определяем дефайн NETWORK_LIB для корректного
# экспорта и импорта классов из DLL
add_definitions(-DNETWORK_LIB)

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Network REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp" "./src/*.c")

# Компиляция библиотеки
add_library (${TARGET} SHARED)

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

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Network)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} Journal)
