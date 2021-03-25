file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(default-coupling SHARED ${SOURCES})

target_include_directories(default-coupling PUBLIC include)

target_link_libraries(default-coupling CfgReader)

target_link_libraries(default-coupling physics)

target_link_libraries(default-coupling coupling)

set_target_properties( default-coupling
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(default-coupling Qt5::Core Qt5::Xml)
