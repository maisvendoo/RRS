file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(modbus SHARED ${SOURCES})

target_include_directories(modbus PUBLIC include)

target_include_directories(modbus PRIVATE ../solver/include)

target_link_libraries(modbus CfgEditor)

target_link_libraries(modbus physics)

target_link_libraries(modbus device)

target_link_libraries(modbus filesystem)

set_target_properties( modbus
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../plugins"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(modbus Qt5::Core Qt5::Xml Qt5::SerialBus)
