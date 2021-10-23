file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(krm130 SHARED ${SOURCES})

target_include_directories(krm130 PUBLIC include)

target_include_directories(device PRIVATE ../solver/include)

target_link_libraries(krm130 CfgReader)

target_link_libraries(krm130 physics)

target_link_libraries(krm130 device)

target_link_libraries(krm130 filesystem)

set_target_properties( krm130
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(krm130 Qt5::Core Qt5::Xml)
