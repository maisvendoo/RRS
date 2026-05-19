# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET model)

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

# Определяем дефайн MODEL_LIB для корректного
# экспорта и импорта классов из DLL
add_definitions(-DMODEL_LIB)

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Xml REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

if(WIN32 OR APPLE)
    find_package(lua CONFIG REQUIRED)
else()
    find_package(Lua 5.4 REQUIRED)
endif()

# Компиляция библиотеки
add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})
if(WIN32 OR APPLE)
    target_link_libraries(${TARGET} lua::lua)
else()
    target_link_libraries(${TARGET} ${LUA_LIBRARIES})
    target_include_directories(${TARGET} PRIVATE ${LUA_INCLUDE_DIR})
endif()

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../train/include)
target_include_directories (${TARGET} PRIVATE ../scenario-manager/include)
# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Xml)
# и со своими не забываем
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} physics)
target_link_libraries (${TARGET} vehicle)
target_link_libraries (${TARGET} solver)
target_link_libraries (${TARGET} train)
target_link_libraries (${TARGET} network)
target_link_libraries (${TARGET} scenario-manager)
target_link_libraries (${TARGET} topology)
target_link_libraries (${TARGET} core)

