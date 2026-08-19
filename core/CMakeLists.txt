cmake_minimum_required(VERSION 3.23)

project(core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_definitions(-DMODULE_LIB)

add_library(core STATIC)

target_sources(core
    PRIVATE
        src/load_module.cpp
        src/string_funcs.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/core/get_module.h
            include/core/load_module.h
            include/core/module_export.h
            include/core/string_funcs.h
)

target_link_libraries(core PRIVATE
    Qt6::Core
)

install(
    TARGETS core
    EXPORT coreTargets
    ARCHIVE DESTINATION sdk/lib
    FILE_SET HEADERS DESTINATION sdk/include
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "coreConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    EXPORT coreTargets
    FILE coreTargets.cmake
    DESTINATION sdk/lib/cmake/core
)

install(
    FILES
        "coreConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/coreConfigVersion.cmake"
    DESTINATION sdk/lib/cmake/core
)

if(MSVC)
    target_compile_options(core PRIVATE /W3)
else()
    target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic)
endif()
