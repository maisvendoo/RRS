file(GLOB SOURCES
    include/*.h
    src/*.cpp
    ../tep70-equipment/include/*.h
    ../tep70-equipment/src/*.cpp
)

add_library(tep70 SHARED ${SOURCES})

target_include_directories(tep70 PRIVATE include)

target_link_libraries(tep70 CfgReader)

target_link_libraries(tep70 physics)

target_link_libraries(tep70 vehicle)

target_link_libraries(tep70 device)

target_link_libraries(tep70 filesystem)

target_link_libraries(tep70 Journal)

target_include_directories(tep70 PRIVATE ../../../simulator/solver/include)

target_include_directories(tep70 PRIVATE ../tep70-equipment/include)

target_include_directories(tep70 PRIVATE ../../../simulator/physics/include)

set_target_properties( tep70
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/tep70"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/tep70"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/tep70"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(tep70 Qt5::Core Qt5::Xml)
