cmake_minimum_required(VERSION 3.10)

set (TARGET viewer2)

project (${TARGET})

set (EXECUTABLE_OUTPUT_PATH "../../bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vsg_DIR /home/ksv/Install/lib/cmake/vsg)
set(vsgXchange_DIR /home/ksv/Install/lib/cmake/vsgXchange)

find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

file(GLOB ${TARGET}_SOURCES "include/*.h" "src/*.cpp")

add_executable(${TARGET} ${${TARGET}_SOURCES})

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
