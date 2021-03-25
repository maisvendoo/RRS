file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(sim-client ${SOURCES})

target_include_directories(sim-client PUBLIC include)

target_link_libraries(sim-client TcpConnection)

set_target_properties( sim-client
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(sim-client Qt5::Core Qt5::Xml)
