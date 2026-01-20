cmake_minimum_required(VERSION 3.23)

set(TARGET route_editor)

project(${TARGET})

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

# TODO: Think how to replace
set(EXECUTABLE_OUTPUT_PATH "../../bin")

# find_package(Qt6 COMPONENTS Widgets Xml REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)

add_executable(${TARGET})

target_sources(${TARGET}
    PRIVATE
        src/Action.cpp
        src/CameraHandler.cpp
        src/EditorGui.cpp
        src/Gizmo.cpp
        src/IntersectionHandler.cpp
        src/KeyboardHandler.cpp
        src/main.cpp
        src/MouseHandler.cpp
        src/ObjectSelector.cpp
        src/Outline.cpp
        src/Route.cpp
        src/RouteEditor.cpp
        src/SceneGraph.cpp
        src/Settings.cpp
        src/shader_funcs.cpp
        src/WindowHandler.cpp

    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/Action.h
            include/CameraHandler.h
            include/EditorGui.h
            include/EditorState.h
            include/Gizmo.h
            include/IntersectionHandler.h
            include/KeyboardHandler.h
            include/KeyStates.h
            include/Mask.h
            include/MouseButton.h
            include/MouseHandler.h
            include/ObjectProperties.h
            include/ObjectSelector.h
            include/ObjectState.h
            include/Outline.h
            include/PagedLodMap.h
            include/Route.h
            include/RouteEditor.h
            include/RouteMap.h
            include/SceneGraph.h
            include/Settings.h
            include/shader_funcs.h
            include/StringMap.h
            include/Vec3Pair.h
            include/WindowHandler.h
)

target_link_libraries(${TARGET}
    PRIVATE
        # Qt6::Widgets
        # Qt6::Xml
        vsg::vsg
        vsgXchange::vsgXchange
        vsgImGui::vsgImGui
        filesystem
        CfgReader
        topology
        physics
        device
)

target_include_directories(${TARGET}
    PRIVATE
        ../common-headers
)

if(MSVC)
    target_compile_options(${TARGET}
        PRIVATE
            /W3
    )
else()
    target_compile_options(${TARGET}
        PRIVATE
            -Wall -Wextra -Wpedantic
    )
endif()
