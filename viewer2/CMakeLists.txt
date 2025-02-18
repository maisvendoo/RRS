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

#set(vsg_DIR /home/ksv/Install/lib/cmake/vsg)
#set(vsgXchange_DIR /home/ksv/Install/lib/cmake/vsgXchange)
#set(vsgImGui_DIR /home/ksv/Install/lib/cmake/vsgImGui)
#set(vsgQt_DIR /home/ksv/Install/lib/cmake/vsgQt)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
# find_package(vsgImGui REQUIRED)
# find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets REQUIRED)
# find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets REQUIRED)
# find_package(vsgQt REQUIRED)

file(GLOB TARGET_SOURCES include/*.h src/*.cpp)
file(GLOB PUGIXML_SOURCES ./dependencies/pugixml/src/*.cpp)

#if (WIN32)
#    add_library(pugixml STATIC IMPORTED)
#    set_target_properties(pugixml PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/dependencies/pugixml.lib)
#else()
#    find_package(pugixml REQUIRED)
#endif()

add_executable(${TARGET} ${TARGET_SOURCES} ${PUGIXML_SOURCES})

target_link_libraries(${TARGET} PRIVATE
    OpenAL::OpenAL
    Qt6::Widgets
    vsg::vsg
    vsgXchange::vsgXchange
    # vsgImGui::vsgImGui
    # vsgQt::vsgQt
    filesystem
    network    
)

# message()

target_include_directories(${TARGET} PRIVATE
    ../simulator/topology/include
    ../common-headers
    ../filesystem/include
    ../network/include
    ./include
    ./dependencies/pugixml/include
)
