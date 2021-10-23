file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(display ${SOURCES})

target_include_directories(display PUBLIC include)

target_include_directories(display PRIVATE ../../common-headers)

set_target_properties( display
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(display Qt5::Widgets)
