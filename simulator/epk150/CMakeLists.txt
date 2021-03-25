file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(epk150 SHARED ${SOURCES})

target_include_directories(epk150 PUBLIC include)

target_include_directories(epk150 PRIVATE ../solver/include)

target_link_libraries(epk150 CfgReader)

target_link_libraries(epk150 physics)

target_link_libraries(epk150 device)

target_link_libraries(epk150 filesystem)

set_target_properties( epk150
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(epk150 Qt5::Core Qt5::Xml)
