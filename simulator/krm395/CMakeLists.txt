file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(krm395 SHARED ${SOURCES})

target_include_directories(krm395 PUBLIC include)

target_include_directories(krm395 PRIVATE ../solver/include)

target_link_libraries(krm395 CfgReader)

target_link_libraries(krm395 physics)

target_link_libraries(krm395 device)

target_link_libraries(krm395 filesystem)

set_target_properties( krm395
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(krm395 Qt5::Core Qt5::Xml)
