file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(solver ${SOURCES})

target_include_directories(solver PUBLIC include)

set_target_properties( solver
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(solver Qt5::Core)
