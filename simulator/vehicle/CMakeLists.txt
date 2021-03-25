file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(vehicle ${SOURCES})

target_include_directories(vehicle PUBLIC include)

target_include_directories(vehicle PRIVATE ../solver/include)

target_include_directories(vehicle PRIVATE ../../common-headers/include)

target_link_libraries(vehicle CfgReader)

target_link_libraries(vehicle physics)

target_link_libraries(vehicle device)

target_link_libraries(vehicle Journal)

set_target_properties( vehicle
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(vehicle Qt5::Core Qt5::Xml)
