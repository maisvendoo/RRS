file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(kvt254 SHARED ${SOURCES})

target_include_directories(kvt254 PUBLIC include)

target_include_directories(kvt254 PRIVATE ../solver/include)

target_link_libraries(kvt254 CfgReader)

target_link_libraries(kvt254 physics)

target_link_libraries(kvt254 device)

target_link_libraries(kvt254 filesystem)

set_target_properties( kvt254
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(kvt254 Qt5::Core Qt5::Xml)
