file(GLOB SOURCES
    include/*.h
    src/*.cpp
    ../vl60-equipment/include/*.h
    ../vl60-equipment/src/*.cpp
)

add_library(vl60k SHARED ${SOURCES})

target_include_directories(vl60k PRIVATE include)

target_link_libraries(vl60k CfgReader)

target_link_libraries(vl60k physics)

target_link_libraries(vl60k vehicle)

target_link_libraries(vl60k device)

target_link_libraries(vl60k filesystem)

target_link_libraries(vl60k Journal)

target_include_directories(vl60k PRIVATE ../../../simulator/solver/include)

target_include_directories(vl60k PRIVATE ../vl60-equipment/include)

target_include_directories(vl60k PRIVATE ../../../simulator/physics/include)

set_target_properties( vl60k
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/vl60k"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/vl60k"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/vl60k"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(vl60k Qt5::Core Qt5::Xml)
