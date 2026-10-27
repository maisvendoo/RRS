cmake_minimum_required(VERSION 3.23)

set (TARGET cassette)

project (${TARGET})

set (LIBRARY_OUTPUT_PATH "../../../lib")
set (CMAKE_SHARED_LIBRARY_PREFIX "")

find_package(Qt6 COMPONENTS Core REQUIRED)

# STATIC: самодостаточное ядро (crypto/format) без экспорта макросов
add_library(${TARGET} STATIC)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

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

target_link_libraries (${TARGET} PUBLIC Qt6::Core CfgReader Journal)
