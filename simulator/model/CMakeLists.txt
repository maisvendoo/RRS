file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(model ${SOURCES})

target_include_directories(model PUBLIC include)

target_link_libraries(model CfgReader)
target_link_libraries(model filesystem)
target_link_libraries(model Journal)
target_link_libraries(model physics)
target_link_libraries(model vehicle)
target_link_libraries(model coupling)
target_link_libraries(model solver)
target_link_libraries(model train)
target_link_libraries(model brakepipe)
target_link_libraries(model TcpConnection)
target_link_libraries(model sim-client)
target_link_libraries(model profile)
target_link_libraries(model device)

target_link_libraries(model asound)
target_link_libraries(model sound-manager)

target_include_directories(model PRIVATE ../../common-headers)

target_include_directories(model PRIVATE ${OPENAL_INCLUDE_DIR})

set_target_properties( model
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(model Qt5::Core Qt5::Xml Qt5::Network)
