cmake_minimum_required(VERSION 3.23)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET player)

# Задаем имя проекта
project (${TARGET})

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Определяем дефайн PLAYER_LIB для корректного
# экспорта и импорта классов из DLL
add_definitions(-DPLAYER_LIB)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеки
# STATIC: экспортных макросов в заголовках нет - на Windows DLL не
# слинкуется, статическая библиотека решает проблему
add_library(${TARGET} STATIC)

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

# Пешая ходьба использует мир коллизий (raycast) и чтение конфига [Player]
target_link_libraries (${TARGET} PUBLIC collision CfgReader)

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
