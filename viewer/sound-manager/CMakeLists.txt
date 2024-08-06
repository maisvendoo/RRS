# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET sound-manager)

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
add_definitions(-DSOUND_MANAGER_LIB)

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Xml REQUIRED)
find_package(OpenAL REQUIRED CONFIG)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеке
add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../../CfgReader/include)
target_include_directories (${TARGET} PRIVATE ../../filesystem/include)
target_include_directories (${TARGET} PRIVATE ../../libJournal/include)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} OpenAL::OpenAL)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} Journal)
