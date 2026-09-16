cmake_minimum_required(VERSION 3.23)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET weather)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метаобъектного компилятора
set(CMAKE_AUTOMOC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Определяем дефайн WEATHER_LIB для корректного
# экспорта и импорта классов из DLL
add_definitions(-DWEATHER_LIB)

# Ищем зависимости Qt6
find_package(Qt6 COMPONENTS Core Xml REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеки
add_library(${TARGET} SHARED)

target_sources(${TARGET}
    PRIVATE
        ${SOURCES}

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            ${INCLUDES}
)

# Компонуемся с библиотеками
target_link_libraries (${TARGET} PUBLIC Qt6::Core)
target_link_libraries (${TARGET} PUBLIC Qt6::Xml)
target_link_libraries (${TARGET} PUBLIC CfgReader)
target_link_libraries (${TARGET} PUBLIC Journal)

install(TARGETS ${TARGET}
    EXPORT ${TARGET}Targets
    FILE_SET HEADERS DESTINATION sdk/include
    LIBRARY DESTINATION sdk/lib
    ARCHIVE DESTINATION sdk/lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION sdk/include)

include (CMakePackageConfigHelpers)

write_basic_package_version_file (
    "${TARGET}ConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install (EXPORT ${TARGET}Targets
    FILE ${TARGET}Targets.cmake
    DESTINATION sdk/lib/cmake/${TARGET})

install (FILES "${TARGET}Config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake"
    DESTINATION sdk/lib/cmake/${TARGET})
