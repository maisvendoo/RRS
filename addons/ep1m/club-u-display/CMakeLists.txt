set (TARGET club-u-display)

project (${TARGET})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set (LIBRARY_OUTPUT_PATH "../../../../modules/ep1m")

set (CMAKE_SHARED_MODULE_PREFIX "")

find_package(Qt6 COMPONENTS Xml Gui Widgets OpenGL REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")
file (GLOB RESOURCES "./resources/*.qrc")

add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES} ${RESOURCES})

target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../ep1m/include)
target_include_directories (${TARGET} PRIVATE ../ep1m-equipment/include)
target_include_directories (${TARGET} PRIVATE ../../../common-headers)
target_include_directories (${TARGET} PRIVATE ../../../core/include)

target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} Qt6::Gui)
target_link_libraries (${TARGET} Qt6::Widgets)
target_link_libraries (${TARGET} Qt6::OpenGL)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} display)
target_link_libraries (${TARGET} core)
target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} filesystem)
