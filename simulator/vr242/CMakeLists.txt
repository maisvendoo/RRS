file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(vr242 SHARED ${SOURCES})

target_include_directories(vr242 PUBLIC include)

target_include_directories(vr242 PRIVATE ../solver/include)

target_link_libraries(vr242 CfgReader)

target_link_libraries(vr242 physics)

target_link_libraries(vr242 device)

target_link_libraries(vr242 filesystem)

set_target_properties( vr242
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(vr242 Qt5::Core Qt5::Xml)
