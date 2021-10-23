file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(coupling ${SOURCES})

target_include_directories(coupling PUBLIC include)

target_link_libraries(coupling CfgReader)

target_link_libraries(coupling physics)

target_link_libraries(coupling Journal)

set_target_properties( coupling
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(coupling Qt5::Core Qt5::Xml)
