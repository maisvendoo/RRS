file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(kvt224 SHARED ${SOURCES})

target_include_directories(kvt224 PUBLIC include)

target_include_directories(kvt224 PRIVATE ../solver/include)

target_link_libraries(kvt224 CfgReader)

target_link_libraries(kvt224 physics)

target_link_libraries(kvt224 device)

target_link_libraries(kvt224 filesystem)

set_target_properties( kvt224
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(kvt224 Qt5::Core Qt5::Xml)
