cmake_minimum_required (VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (RRS)

add_subdirectory (CfgReader)
add_subdirectory (CfgEditor)
add_subdirectory (filesystem)
add_subdirectory (libJournal)
add_subdirectory (tcp-connection)

add_subdirectory (simulator)

add_subdirectory (viewer)

add_subdirectory (addons/passcar)
add_subdirectory (addons/freightcar)
add_subdirectory (addons/vl60)

