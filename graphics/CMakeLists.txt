cmake_minimum_required(VERSION 3.23)

project(graphics)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

set(LIBRARY_OUTPUT_PATH "../../lib")

add_library(graphics STATIC)

target_sources(graphics
    PRIVATE
        src/common.cpp
        src/shader_funcs.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/graphics/common.h
            include/graphics/shader_funcs.h
)

target_link_libraries(graphics PRIVATE
    Qt6::Core
    vsg::vsg
    vsgXchange::vsgXchange
    Journal
    filesystem
)

if(WIN32)
    target_compile_options(graphics PRIVATE /W3)
else()
    target_compile_options(graphics PRIVATE -Wall -Wextra -Wpedantic)
endif()
