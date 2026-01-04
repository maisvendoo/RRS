set(TARGET scenario-manager)

project(${TARGET})

set(CMAKE_AUTOMOC ON)

set (LIBRARY_OUTPUT_PATH "../../../lib")

set (CMAKE_SHARED_LIBRARY_PREFIX "")

add_definitions(-DSCNMGR_LIB)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(lua CONFIG REQUIRED)
find_package(sol2 REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

target_include_directories (${TARGET} PRIVATE ./include/)

target_link_libraries(${TARGET} Qt6::Core)
target_link_libraries(${TARGET} lua::lua)
target_link_libraries(${TARGET} sol2)
