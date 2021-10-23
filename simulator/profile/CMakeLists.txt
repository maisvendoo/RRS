file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(profile ${SOURCES})

target_include_directories(profile PUBLIC include)

target_include_directories(profile PRIVATE ../../common-headers/include)

target_link_libraries(profile filesystem)

target_link_libraries(profile Journal)

set_target_properties( profile
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(profile Qt5::Core)
