cmake_minimum_required (VERSION 3.29)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (RRS VERSION 1.8.0)

add_subdirectory (CfgReader)
add_subdirectory (CfgEditor)
add_subdirectory (filesystem)
add_subdirectory (libJournal)
add_subdirectory (network)

add_subdirectory (simulator)

add_subdirectory (sound-manager)
add_subdirectory (viewer)

add_subdirectory (route_editor)

add_subdirectory (addons/passcar)
add_subdirectory (addons/freightcar)
add_subdirectory (addons/vl60)

add_subdirectory (launcher)

add_subdirectory (tools/dmd2gltf)
add_subdirectory (tools/profconv)
add_subdirectory (tools/pathconv)
add_subdirectory (tools/offset-parallel-gen)
add_subdirectory (tools/offset-spline-gen)
add_subdirectory (tools/topologycheck)
add_subdirectory (tools/routeconv)
add_subdirectory (tools/route-map)
add_subdirectory (tools/route-transform)
