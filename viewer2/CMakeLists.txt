cmake_minimum_required(VERSION 3.10)

set (TARGET viewer2)

project (${TARGET})

set (EXECUTABLE_OUTPUT_PATH "../../bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies/WIN32)
else()
    set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies/UNIX)
endif()

file(GLOB PUGIXML_SOURCES ${DEPENDENCIES_DIR}/pugixml-1.15/src/*)

set(vsg_DIR /home/ksv/Install/lib/cmake/vsg)
set(vsgXchange_DIR /home/ksv/Install/lib/cmake/vsgXchange)

find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

file(GLOB TARGET_SOURCES include/*.h src/*.cpp)

add_executable(${TARGET} ${PUGIXML_SOURCES} ${TARGET_SOURCES})

target_link_libraries(${TARGET} PRIVATE
    vsg::vsg
    vsgXchange::vsgXchange
    filesystem
    network
)

target_include_directories(${TARGET} PRIVATE
    ../filesystem/include
    ../network/include
    ./include
)
