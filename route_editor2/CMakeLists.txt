cmake_minimum_required(VERSION 3.23)

project(route_editor2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)

set(EXECUTABLE_OUTPUT_PATH "../../bin")

add_executable(route_editor2)

file(GLOB_RECURSE INCLUDES "include/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")

target_sources(route_editor2
    PRIVATE
        ${SOURCES}

    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            ${INCLUDES}
)

target_link_libraries(route_editor2
    PRIVATE
        Qt6::Core
        vsg::vsg
        vsgXchange::vsgXchange
        vsgImGui::vsgImGui
        CfgReader
        Journal
        # core
        filesystem
        graphics
        topology
)

if(MSVC)
    target_compile_options(route_editor2 PRIVATE /W3)
else()
    target_compile_options(route_editor2 PRIVATE -Wall -Wextra -Wpedantic)
endif()
