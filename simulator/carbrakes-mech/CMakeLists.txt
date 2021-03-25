file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(carbrakes-mech SHARED ${SOURCES})

target_include_directories(carbrakes-mech PUBLIC include)

target_include_directories(carbrakes-mech PRIVATE ../solver/include)

target_link_libraries(carbrakes-mech CfgReader)

target_link_libraries(carbrakes-mech physics)

target_link_libraries(carbrakes-mech device)

target_link_libraries(carbrakes-mech filesystem)

set_target_properties( carbrakes-mech
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(carbrakes-mech Qt5::Core Qt5::Xml)
