# Минимальная требуемая версия CMake
cmake_minimum_required (VERSION 3.7)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET Journal)

# Задаем имя проекта
project (${TARGET})

# Qt-шное барахло для запуска метообъектного компилятора
# сборки ресурсов и генерации кода UI при работе с формами
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Ищем зависимости Qt5
find_package(Qt5 COMPONENTS Core REQUIRED)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB CPP_SOURCES "./src/*.cpp")
file (GLOB C_SOURCES "./src/*.c")

# Компиляция библиотеке
add_library (${TARGET} SHARED ${CPP_SOURCES} ${C_SOURCES} ${INCLUDES})

# Указываем пути поиска заголовков
target_include_directories (${TARGET} PRIVATE ./include/)

if(WIN32)
    target_link_libraries(${TARGET} Qt5::Core ws2_32)
else()
    target_link_libraries(${TARGET} Qt5::Core)
endif()

if (WIN32)

    install(TARGETS ${TARGET}
    EXPORT ${TARGET}Targets
    FILE_SET HEADERS
    LIBRARY DESTINATION sdk/lib
    ARCHIVE DESTINATION sdk/lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION sdk/include)

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${TARGET}ConfigVersion.cmake"
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion)

    install(EXPORT ${TARGET}Targets
    FILE ${TARGET}Targets.cmake
    DESTINATION sdk/lib/cmake/${TARGET})

    install(FILES "${TARGET}Config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake"
        DESTINATION sdk/lib/cmake/${TARGET})

    install(FILES ${INCLUDES} DESTINATION sdk/include)

else()

endif()
