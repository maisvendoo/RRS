file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(device ${SOURCES})

target_include_directories(device PUBLIC include)

target_include_directories(device PRIVATE ../solver/include)

target_include_directories(device PRIVATE ../../common-headers/include)

target_link_libraries(device CfgReader)

target_link_libraries(device physics)

target_link_libraries(device filesystem)

target_link_libraries(device Journal)

target_link_libraries(device solver)

set_target_properties( device
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(device Qt5::Core Qt5::Xml)
