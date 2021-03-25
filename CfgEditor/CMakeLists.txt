file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(CfgEditor ${SOURCES})

target_include_directories(CfgEditor PUBLIC include)

set_target_properties( CfgEditor
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(CfgEditor Qt5::Core)
target_link_libraries(CfgEditor Qt5::Xml)
