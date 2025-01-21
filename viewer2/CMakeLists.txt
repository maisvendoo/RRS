cmake_minimum_required(VERSION 3.10)

set (TARGET viewer2)

project (${TARGET})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set (EXECUTABLE_OUTPUT_PATH "../../bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenAL REQUIRED CONFIG)

if(WIN32)
    set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies/WIN32)
else()
    set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies/UNIX)
endif()

file(GLOB PUGIXML_SOURCES ${DEPENDENCIES_DIR}/pugixml-1.15/src/*)

set(vsg_DIR /home/ksv/Install/lib/cmake/vsg)
set(vsgXchange_DIR /home/ksv/Install/lib/cmake/vsgXchange)
set(vsgImGui_DIR /home/ksv/Install/lib/cmake/vsgImGui)
set(vsgQt_DIR /home/ksv/Install/lib/cmake/vsgQt)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)
# find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets REQUIRED)
# find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets REQUIRED)
# find_package(vsgQt REQUIRED)

file(GLOB TARGET_SOURCES include/*.h src/*.cpp)

add_executable(${TARGET} ${PUGIXML_SOURCES} ${TARGET_SOURCES})

target_link_libraries(${TARGET} PRIVATE
    OpenAL::OpenAL
    Qt6::Widgets
    vsg::vsg
    vsgXchange::vsgXchange
    vsgImGui::vsgImGui
    # vsgQt::vsgQt
    filesystem
    network
)

target_include_directories(${TARGET} PRIVATE
    ../common-headers
    ../filesystem/include
    ../network/include
    ./include
)
