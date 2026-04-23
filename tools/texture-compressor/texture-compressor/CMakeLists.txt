# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET texcompr)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (EXECUTABLE_OUTPUT_PATH "../../../../bin")

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Core REQUIRED)

find_package(KTX QUIET)
if(NOT KTX_FOUND)
    find_package(ktx QUIET)
endif()
if(NOT KTX_FOUND AND NOT ktx_FOUND)
    message(FATAL_ERROR "KTX library not found. Install libktx v4+ with BASIS & Vulkan support.")
endif()

if(TARGET KTX::ktx)
    set(KTX_TARGET KTX::ktx)
elseif(TARGET ktx::ktx)
    set(KTX_TARGET ktx::ktx)
else()
    message(FATAL_ERROR "Found KTX but could not resolve imported target name.")
endif()

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB CPP_SOURCES "./src/*.cpp")
file (GLOB C_SOURCES "./src/*.c")

# Компиляция приложения
add_executable (${TARGET} ${CPP_SOURCES} ${C_SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)

# Компонуемся с библиотеками Qt6
target_link_libraries (${TARGET} Qt6::Core)
target_link_libraries (${TARGET} ${KTX_TARGET})
