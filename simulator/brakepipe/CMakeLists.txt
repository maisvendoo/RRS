file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(brakepipe ${SOURCES})

target_include_directories(brakepipe PUBLIC include)

target_link_libraries(brakepipe CfgReader)

target_link_libraries(brakepipe physics)

set_target_properties( brakepipe
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(brakepipe Qt5::Core Qt5::Xml)
