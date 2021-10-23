file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(route-loader SHARED ${SOURCES})

target_link_libraries(route-loader filesystem)

target_link_libraries(route-loader library)

target_include_directories(route-loader PRIVATE ../../common-headers)

target_include_directories(route-loader PUBLIC include)



set_target_properties( route-loader
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(route-loader Qt5::Core)
target_link_libraries(route-loader ${OPENSCENEGRAPH_LIBRARIES})
