cmake_minimum_required(VERSION 3.10)

project(viewer2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vsg_DIR /home/ksv/Install/lib/cmake/vsg)
set(vsgXchange_DIR /home/ksv/Install/lib/cmake/vsgXchange)

find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

add_executable(viewer2 main.cpp)

target_link_libraries(viewer2 vsg::vsg vsgXchange::vsgXchange)
