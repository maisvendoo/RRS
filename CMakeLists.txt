cmake_minimum_required (VERSION 3.29)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (RRS VERSION 1.3.0)

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

add_subdirectory (launcher)

add_subdirectory (tools/profconv)
add_subdirectory (tools/pathconv)
add_subdirectory (tools/offset-parallel-gen)
add_subdirectory (tools/offset-spline-gen)
add_subdirectory (tools/routeconv)

