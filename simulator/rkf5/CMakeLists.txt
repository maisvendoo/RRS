file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(rkf5 SHARED ${SOURCES})

target_include_directories(rkf5 PUBLIC include)

target_link_libraries(rkf5 solver)

set_target_properties( rkf5
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(rkf5 Qt5::Core)
