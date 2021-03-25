file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(physics ${SOURCES})

target_include_directories(physics PUBLIC include)

set_target_properties( physics
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(physics Qt5::Core)
