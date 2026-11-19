set (TARGET dispatcher-console)
project (${TARGET})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

set (EXECUTABLE_OUTPUT_PATH "../../../bin")

find_package(Qt6 COMPONENTS Xml REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
    add_executable (${TARGET} WIN32 ${SOURCES} ${INCLUDES})
else()
    add_executable (${TARGET} ${SOURCES} ${INCLUDES})
endif()

target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../common-headers)

target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} Qt6::Gui)
target_link_libraries (${TARGET} Qt6::Widgets)

target_link_libraries (${TARGET} system)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} network)

if (WIN32)
    target_link_libraries (${TARGET} psapi)
endif()