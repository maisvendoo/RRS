file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(library ${SOURCES})

target_include_directories(library PUBLIC include)

target_include_directories(library PRIVATE ../../common-headers)

target_link_libraries(library INTERFACE -ldl)

target_link_libraries(library PRIVATE filesystem)

set_target_properties( library
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(library PRIVATE Qt5::Core)
