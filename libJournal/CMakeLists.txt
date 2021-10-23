file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(Journal ${SOURCES})

target_include_directories(Journal PUBLIC include)

set_target_properties( Journal
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(Journal Qt5::Core)
