file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(osgdb_dmd.bak SHARED ${SOURCES})

target_include_directories(osgdb_dmd.bak PUBLIC include)

target_include_directories(osgdb_dmd.bak PRIVATE ../../common-headers)

set_target_properties( osgdb_dmd.bak
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(osgdb_dmd.bak Qt5::Core)
target_link_libraries(osgdb_dmd.bak ${OPENSCENEGRAPH_LIBRARIES})
