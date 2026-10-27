# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET collider-gen)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
set(CMAKE_AUTOMOC ON)

# Путь, куда надо сложить собранное
set (EXECUTABLE_OUTPUT_PATH "../../../bin")

# Ищем зависимости
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция приложения
add_executable (${TARGET} ${SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)

# Компонуемся с библиотеками
target_link_libraries (${TARGET} Qt6::Core)
target_link_libraries (${TARGET} vsg::vsg)
target_link_libraries (${TARGET} vsgXchange::vsgXchange)
target_link_libraries (${TARGET} collision)
target_link_libraries (${TARGET} graphics)
