# Устанавливаем переменную TARGET равной имени проекта
set (TARGET rolling-stock-creator)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
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

# Компиляция приложения
if (WIN32)
    add_executable (${TARGET} WIN32 ${SOURCES} ${INCLUDES})
else()
    add_executable (${TARGET} ${SOURCES} ${INCLUDES})
endif()

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} Qt6::Gui)
target_link_libraries (${TARGET} Qt6::Widgets)

# и с нашими не забываем
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
