cmake_minimum_required(VERSION 3.23)

# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set (TARGET collision)

# Задаем имя проекта
project (${TARGET})

# Путь, куда надо сложить собранное
set (LIBRARY_OUTPUT_PATH "../../../lib")
# Указываем не добавлять к имени бинарника никаких префиксов
set (CMAKE_SHARED_LIBRARY_PREFIX "")

# Определяем дефайн COLLISION_LIB для корректного
# экспорта и импорта классов из DLL
add_definitions(-DCOLLISION_LIB)

# Модуль - чистый C++ (без Qt): движок коллизий не должен тянуть Qt.
# ВНИМАНИЕ: с optflags (cmake/OptimizationFlags.cmake) модуль НЕ линковать:
# -ffinite-math-only ломает NaN-проверки внутри Jolt.

#------------------------------------------------------------------------------
# Jolt Physics через FetchContent
#------------------------------------------------------------------------------

# Версия Jolt Physics
set (JOLT_GIT_TAG "v5.6.0" CACHE STRING "Тег или коммит Jolt Physics")

# Детерминированная симуляция (медленнее, нужна для сетевой игры/реплеев)
option (RRS_COLLISION_DETERMINISTIC "Кроссплатформенно-детерминированный Jolt" OFF)

include (FetchContent)

# Настройки сборки Jolt: только статическая библиотека.
# Тесты/примеры/viewer Jolt собираются, только когда он корневой проект,
# через FetchContent они отключены автоматически.
# FORCE нужен, т.к. Jolt объявляет эти имена через option()
set (DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set (CROSS_PLATFORM_DETERMINISTIC ${RRS_COLLISION_DETERMINISTIC} CACHE BOOL "" FORCE)
# MinGW + SHARED + LTO = ошибки линковки "cannot export symbol"
set (INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
# Иначе Jolt собирается с -Wall -Werror и падает на чужих компиляторах
set (ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
# Не добавляем правила установки Jolt в наш install
set (ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
# На MSVC линкуемся с динамическим рантаймом, как весь проект
set (USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
# GPU-compute (софт-боди) не используем, только CPU
set (JPH_USE_DX12 OFF CACHE BOOL "" FORCE)
set (JPH_USE_VK OFF CACHE BOOL "" FORCE)
set (JPH_USE_MTL OFF CACHE BOOL "" FORCE)

# Jolt собирается статически и линкуется внутрь SHARED-модуля collision,
# поэтому нужен position-independent code
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# При проблемах с сетью можно указать локальную копию репозитория Jolt:
#   cmake -DFETCHCONTENT_SOURCE_DIR_JOLTPHYSICS=D:/libs/JoltPhysics ...
FetchContent_Declare (
    JoltPhysics
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG        ${JOLT_GIT_TAG}
    GIT_SHALLOW    TRUE
    # CMakeLists.txt у Jolt лежит в подкаталоге Build
    SOURCE_SUBDIR  Build
)

FetchContent_MakeAvailable (JoltPhysics)

# Пути к заголовкам и сорцам
file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

# Компиляция библиотеки
add_library (${TARGET} SHARED)

target_sources (${TARGET}
    PRIVATE
        ${SOURCES}

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            ${INCLUDES}
)

# Пути к общим заголовкам проекта
target_include_directories (${TARGET} PRIVATE ../../common-headers)

# Jolt линкуем PRIVATE: публичные заголовки модуля не включают заголовки
# Jolt, поэтому Jolt не попадает в интерфейс цели и не мешает install(EXPORT)
target_link_libraries (${TARGET} PRIVATE Jolt)

# Установка (по образцу остальных модулей simulator)
install (TARGETS ${TARGET}
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
