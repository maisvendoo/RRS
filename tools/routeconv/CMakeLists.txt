# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET routeconv)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Отключаем автоматическую обработку форм *.ui
set(CMAKE_AUTOUIC OFF)

# Путь, куда надо сложить собранное
set (EXECUTABLE_OUTPUT_PATH "../../../bin")

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Xml REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")
file (GLOB FORMS "./forms/*.ui")
file (GLOB TS_FILES "./translations/*.ts")
file (GLOB RESOURCES "./resources/*.qrc")

# Сами обрабатываем формы
qt_wrap_ui(UI_HEADERS ${FORMS})
# Сообщаем что в каталоге сборки тоже стоит поискать заголовки
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Компиляция приложения
if (WIN32)
    add_executable (${TARGET} WIN32 ${SOURCES} ${INCLUDES} ${UI_HEADERS} ${TS_FILES} ${RESOURCES})
else()
    add_executable (${TARGET} ${SOURCES} ${INCLUDES} ${UI_HEADERS} ${TS_FILES} ${RESOURCES})
endif()

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../../CfgReader/include)
target_include_directories (${TARGET} PRIVATE ../../CfgEditor/include)
target_include_directories (${TARGET} PRIVATE ../../filesystem/include)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} Qt6::Gui)
target_link_libraries (${TARGET} Qt6::Widgets)

# и со своими не забываем
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} CfgEditor)
target_link_libraries (${TARGET} filesystem)
