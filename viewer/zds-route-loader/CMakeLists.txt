file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(zds-route-loader SHARED ${SOURCES})

target_link_libraries(zds-route-loader filesystem)

target_link_libraries(zds-route-loader route-loader)

target_include_directories(zds-route-loader PUBLIC include)

set_target_properties( zds-route-loader
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(zds-route-loader Qt5::Core)
target_link_libraries(zds-route-loader ${OPENSCENEGRAPH_LIBRARIES})
