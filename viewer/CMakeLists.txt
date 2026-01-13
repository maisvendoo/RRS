set (TARGET viewer)

project (${TARGET})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set (EXECUTABLE_OUTPUT_PATH "../../bin")

find_package(Qt6 COMPONENTS Widgets Xml REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)

file(GLOB TARGET_SOURCES include/*.h src/*.cpp)

add_executable(${TARGET} ${TARGET_SOURCES})

add_subdirectory (display)
add_subdirectory (display-player)

target_link_libraries(${TARGET} PRIVATE
    Qt6::Widgets
    Qt6::Xml
    vsg::vsg
    vsgXchange::vsgXchange
    vsgImGui::vsgImGui
    filesystem
    CfgReader
    network
    sound-manager
    display
)

target_include_directories(${TARGET} PRIVATE
    ../simulator/topology/include
    ../simulator/vehicle/include
    ../common-headers
    ../filesystem/include
    ../network/include
    ../sound-manager/include
    display/include
    ./include
)
