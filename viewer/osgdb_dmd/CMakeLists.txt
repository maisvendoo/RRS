# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET osgdb_dmd)

# Задаем имя проекта
project (${TARGET})

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../plugins")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_MODULE_PREFIX "mingw_")

# Ищем зависимости
find_package(OpenSceneGraph REQUIRED osg osgViewer osgDB OpenThreads osgUtil)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеке
add_library (${TARGET} MODULE ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})

target_link_libraries (${TARGET} ${OPENSCENEGRAPH_LIBRARIES})
