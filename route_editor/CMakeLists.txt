cmake_minimum_required(VERSION 3.23)

set(TARGET route_editor)

project(${TARGET})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# TODO: Think how to replace
set(EXECUTABLE_OUTPUT_PATH "../../bin")

# find_package(Qt6 COMPONENTS Widgets Xml REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)

add_executable(${TARGET})

file(GLOB_RECURSE INCLUDES "./include/*.h")
file(GLOB_RECURSE SOURCES "./src/*.cpp")

target_sources(${TARGET}
    PRIVATE
        ${SOURCES}

    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            ${INCLUDES}
)

target_link_libraries(${TARGET}
    PRIVATE
        # Qt6::Widgets
        # Qt6::Xml
        vsg::vsg
        vsgXchange::vsgXchange
        vsgImGui::vsgImGui
        CfgReader
        Journal
        filesystem
        topology
        graphics
        # parsing
)

if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W3)
else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()
