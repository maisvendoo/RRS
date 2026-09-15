set (TARGET ep1m-iocontroller)

project (${TARGET})

set(CMAKE_AUTOMOC ON)

set (LIBRARY_OUTPUT_PATH "../../../../modules/ep1m")

set (CMAKE_SHARED_MODULE_PREFIX "")

find_package(Qt6 COMPONENTS Xml REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../../viewer/IO-controller/include)
target_include_directories (${TARGET} PRIVATE ../../../common-headers)
target_include_directories (${TARGET} PRIVATE ../../../core/include)

target_link_libraries (${TARGET} Qt6::Xml)
target_link_libraries (${TARGET} io-controller)
target_link_libraries (${TARGET} core)
