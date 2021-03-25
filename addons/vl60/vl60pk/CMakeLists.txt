file(GLOB SOURCES
    include/*.h
    src/*.cpp
    ../vl60-equipment/include/*.h
    ../vl60-equipment/src/*.cpp
)

add_library(vl60pk SHARED ${SOURCES})

target_include_directories(vl60pk PRIVATE include)

target_link_libraries(vl60pk CfgReader)

target_link_libraries(vl60pk physics)

target_link_libraries(vl60pk vehicle)

target_link_libraries(vl60pk device)

target_link_libraries(vl60pk filesystem)

target_link_libraries(vl60pk Journal)

target_include_directories(vl60pk PRIVATE ../../../simulator/solver/include)

target_include_directories(vl60pk PRIVATE ../vl60-equipment/include)

target_include_directories(vl60pk PRIVATE ../../../simulator/physics/include)

set_target_properties( vl60pk
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/vl60pk"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/vl60pk"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/vl60pk"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(vl60pk Qt5::Core Qt5::Xml)
