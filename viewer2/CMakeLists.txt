set (TARGET viewer2)

project (${TARGET})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set (EXECUTABLE_OUTPUT_PATH "../../bin")

if (UNIX)
    set(vsg_DIR /home/ksv/Install/lib/cmake/vsg)
    set(vsgXchange_DIR /home/ksv/Install/lib/cmake/vsgXchange)
    set(vsgImGui_DIR /home/ksv/Install/lib/cmake/vsgImGui)
    set(vsgQt_DIR /home/ksv/Install/lib/cmake/vsgQt)
endif()

find_package(Qt6 COMPONENTS Widgets Xml REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)

file(GLOB TARGET_SOURCES include/*.h src/*.cpp)

add_executable(${TARGET} ${TARGET_SOURCES})

target_link_libraries(${TARGET} PRIVATE
    Qt6::Widgets
    Qt6::Xml
    vsg::vsg
    vsgXchange::vsgXchange
    vsgImGui::vsgImGui
    # vsgQt::vsgQt
    filesystem
    CfgReader
    network
    sound-manager
)

target_include_directories(${TARGET} PRIVATE
    ../simulator/topology/include
    ../common-headers
    ../filesystem/include
    ../CfgReader/include
    ../network/include
    ../sound-manager/include
    ./include
)
