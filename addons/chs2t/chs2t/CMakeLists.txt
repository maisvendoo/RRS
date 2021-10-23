file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(chs2t SHARED ${SOURCES})

target_include_directories(chs2t PRIVATE include)

target_link_libraries(chs2t CfgReader)

target_link_libraries(chs2t physics)

target_link_libraries(chs2t vehicle)

target_link_libraries(chs2t device)

target_link_libraries(chs2t filesystem)

target_link_libraries(chs2t Journal)

target_include_directories(chs2t PRIVATE ../../../simulator/solver/include)

target_include_directories(chs2t PRIVATE ../../../simulator/physics/include)

set_target_properties( chs2t
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/chs2t"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/chs2t"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../modules/chs2t"
    PREFIX ""
    DEBUG_POSTFIX ""
)

target_link_libraries(chs2t Qt5::Core Qt5::Xml)
