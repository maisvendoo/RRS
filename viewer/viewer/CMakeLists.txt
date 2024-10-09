# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET viewer)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set (EXECUTABLE_OUTPUT_PATH "../../../bin")

# Ищем зависимости
find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)
find_package(OpenSceneGraph REQUIRED osg osgViewer osgDB OpenThreads osgGA osgUtil osgText)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")
file (GLOB IMGUI_INCLUDES "../imgui/include/*.h")
file (GLOB IMGUI_SOURCES "../imgui/src/*.cpp")

# Компиляция библиотеке
add_executable (${TARGET} ${SOURCES} ${INCLUDES} ${IMGUI_SOURCES} ${IMGUI_INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../../simulator/vehicle/include)
target_include_directories (${TARGET} PRIVATE ../../filesystem/include)
target_include_directories (${TARGET} PRIVATE ../../network/include)
target_include_directories (${TARGET} PRIVATE ../route-loader/include)
target_include_directories (${TARGET} PRIVATE ../library/include)
target_include_directories (${TARGET} PRIVATE ../display/include)

target_include_directories (${TARGET} PRIVATE ../sound-manager/include)
target_include_directories (${TARGET} PRIVATE ../../asound/include)

target_include_directories (${TARGET} PRIVATE ../imgui/include)

target_include_directories (${TARGET} PRIVATE ../../simulator/topology/include)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Gui)
target_link_libraries (${TARGET} Qt6::Widgets)
target_link_libraries (${TARGET} Qt6::Network)
target_link_libraries (${TARGET} Qt::OpenGLWidgetsPrivate)
target_link_libraries (${TARGET} ${OPENSCENEGRAPH_LIBRARIES})
target_link_libraries (${TARGET} route-loader)
target_link_libraries (${TARGET} library)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} network)
target_link_libraries (${TARGET} display)
target_link_libraries (${TARGET} sound-manager)

