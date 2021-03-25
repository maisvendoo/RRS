file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(sound-manager ${SOURCES})

target_include_directories(sound-manager PUBLIC include)

target_link_libraries(sound-manager CfgReader)

target_link_libraries(sound-manager asound)

target_link_libraries(sound-manager filesystem)

target_link_libraries(sound-manager Journal)

target_include_directories(sound-manager PRIVATE ${OPENAL_INCLUDE_DIR})

set_target_properties( sound-manager
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(sound-manager Qt5::Core Qt5::Xml)
