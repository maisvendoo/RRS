file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(ef-coupling SHARED ${SOURCES})

target_include_directories(ef-coupling PUBLIC include)

target_link_libraries(ef-coupling CfgReader)

target_link_libraries(ef-coupling physics)

target_link_libraries(ef-coupling coupling)

set_target_properties( ef-coupling
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(ef-coupling Qt5::Core Qt5::Xml)
