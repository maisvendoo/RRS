set (TARGET session)

project (${TARGET})

set(CMAKE_AUTOMOC ON)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_library (${TARGET} STATIC ${SOURCES} ${INCLUDES})

set_target_properties (${TARGET} PROPERTIES OUTPUT_NAME session)

target_include_directories (${TARGET} PUBLIC ./include/)
target_include_directories (${TARGET} PRIVATE ../../common-headers)

find_package(Qt6 COMPONENTS Core Xml REQUIRED)
target_link_libraries (${TARGET} PUBLIC Qt6::Core Qt6::Xml)

