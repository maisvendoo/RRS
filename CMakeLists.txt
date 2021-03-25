cmake_minimum_required(VERSION 3.7)

project(RRS
        VERSION 2.0.0
        DESCRIPTION "Russian Railway Simulator"
        LANGUAGES CXX C
)

# build all examples into the bin directory
set(CMAKE_DEBUG_POSTFIX _d)

# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# set the use of C++17 globally
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Xml REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS SerialBus REQUIRED)
#find_package(Qt6 COMPONENTS Core5Compat REQUIRED)

# add clobber build target to clear all the non git registered files/directories
add_custom_target(clobber
    COMMAND git clean -d -f -x
)

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)

find_package(OpenSceneGraph COMPONENTS osgUtil osgDB osgText osgGA osgFX osgSim osgViewer REQUIRED)
#find_library(OSG_QT_LIB osgQOpenGL REQUIRED)


add_subdirectory(tcp-connection)
add_subdirectory(CfgReader)
add_subdirectory(CfgEditor)
add_subdirectory(filesystem)
add_subdirectory(libJournal)
add_subdirectory(asound)
add_subdirectory(osgdb_dmd)
add_subdirectory(simulator)
add_subdirectory(viewer)

add_subdirectory(addons/chs2t/chs2t)
add_subdirectory(addons/passcar/passcar)
add_subdirectory(addons/tep70/tep70)
add_subdirectory(addons/vl60/vl60k)
add_subdirectory(addons/vl60/vl60pk)

add_subdirectory(tools/profconv)
add_subdirectory(tools/pathconv)
add_subdirectory(tools/routeconv)
add_subdirectory(launcher2)
add_subdirectory(launcher)
