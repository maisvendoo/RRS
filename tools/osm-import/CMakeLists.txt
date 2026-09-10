set (TARGET osm-import)

project (${TARGET})

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Xml Network REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_executable (${TARGET} ${SOURCES} ${INCLUDES})

target_include_directories (${TARGET} PRIVATE ./include/)

target_link_libraries (${TARGET} Qt6::Core)
target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} Qt6::Network)
