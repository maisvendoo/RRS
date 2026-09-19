# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET server-daemon)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (EXECUTABLE_OUTPUT_PATH "../../../bin")

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Xml REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция приложения
add_executable (${TARGET} ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Core)
target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} Qt6::Network)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
