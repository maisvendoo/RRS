# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET sa3)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../modules")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_MODULE_PREFIX "")

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Xml REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеке
add_library (${TARGET} MODULE ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../physics/include)
target_include_directories (${TARGET} PRIVATE ../solver/include)
target_include_directories (${TARGET} PRIVATE ../device/include)
target_include_directories (${TARGET} PRIVATE ../../CfgReader/include)
target_include_directories (${TARGET} PRIVATE ../../filesystem/include)
target_include_directories (${TARGET} PRIVATE ../../common-headers)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} physics)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} filesystem)
