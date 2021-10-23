file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(display-player ${SOURCES})

target_include_directories(display-player PUBLIC include)

target_include_directories(display-player PRIVATE ../../common-headers)

target_link_libraries(display-player CfgReader)

target_link_libraries(display-player display)

set_target_properties( display-player
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(display-player Qt5::Core)
