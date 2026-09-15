set (TARGET ep1m-autopilot)

project (${TARGET})

set(CMAKE_AUTOMOC ON)

set (LIBRARY_OUTPUT_PATH "../../../../modules/ep1m")

set (CMAKE_SHARED_MODULE_PREFIX "")

find_package(Qt6 COMPONENTS Xml REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../ep1m-equipment/include/)
target_include_directories (${TARGET} PRIVATE ../../../common-headers)

target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} CfgReader)
target_link_libraries (${TARGET} physics)
target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} vehicle)
target_link_libraries (${TARGET} core)
