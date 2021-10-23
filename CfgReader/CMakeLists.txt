file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(CfgReader ${SOURCES})

target_include_directories(CfgReader PUBLIC include)

set_target_properties( CfgReader
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(CfgReader Qt5::Core)
target_link_libraries(CfgReader Qt5::Xml)

