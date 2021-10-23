file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(evr305 SHARED ${SOURCES})

target_include_directories(evr305 PUBLIC include)

target_include_directories(evr305 PRIVATE ../solver/include)

target_link_libraries(evr305 CfgReader)

target_link_libraries(evr305 physics)

target_link_libraries(evr305 device)

target_link_libraries(evr305 filesystem)

set_target_properties( evr305
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)


target_link_libraries(evr305 Qt5::Core Qt5::Xml)
