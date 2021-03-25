file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(passcar SHARED ${SOURCES})

target_include_directories(passcar PRIVATE include)

target_link_libraries(passcar CfgReader)

target_link_libraries(passcar physics)

target_link_libraries(passcar vehicle)

target_link_libraries(passcar device)

target_link_libraries(passcar filesystem)

target_link_libraries(passcar Journal)

target_include_directories(passcar PRIVATE ../../../simulator/solver/include)

target_include_directories(passcar PRIVATE ../../../simulator/physics/include)

set_target_properties( passcar
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/passcar"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/passcar"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/passcar"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(passcar Qt5::Core Qt5::Xml)
