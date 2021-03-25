file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(rk4 SHARED ${SOURCES})

target_include_directories(rk4 PUBLIC include)

target_link_libraries(rk4 solver)

set_target_properties( rk4
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(rk4 Qt5::Core)
