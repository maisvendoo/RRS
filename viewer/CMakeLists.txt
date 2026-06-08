set (TARGET viewer)

project (${TARGET})

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

set (EXECUTABLE_OUTPUT_PATH "../../bin")

find_package(Qt6 COMPONENTS Widgets Xml REQUIRED)
find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)
find_package(vsgImGui REQUIRED)

file(GLOB TARGET_SOURCES include/*.h src/*.cpp)

add_executable(${TARGET} ${TARGET_SOURCES})

# Optimization flags
target_compile_options(${TARGET} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -fno-math-errno
        -fno-trapping-math
    >
    $<$<AND:$<CONFIG:Release,RelWithDebInfo,MinSizeRel>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -O3
        -funroll-loops
        -fno-signed-zeros
    >
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -Og
    >
)

add_subdirectory (display)
add_subdirectory (display-player)
add_subdirectory (IO-controller)

target_link_libraries(${TARGET} PRIVATE
    Qt6::Widgets
    Qt6::Xml
    vsg::vsg
    vsgXchange::vsgXchange
    vsgImGui::vsgImGui
    filesystem
    CfgReader
    network
    sound-manager
    display
    topology
    parsing
    graphics
    core
)

target_include_directories(${TARGET} PRIVATE
    ../common-headers
    ../sound-manager/include
    display/include
    ./include
)
