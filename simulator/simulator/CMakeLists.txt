file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_executable(simulator ${SOURCES})

target_include_directories(simulator PUBLIC include)

target_include_directories(simulator PRIVATE ../../common-headers)

target_link_libraries(simulator CfgReader)
target_link_libraries(simulator filesystem)
target_link_libraries(simulator Journal)
target_link_libraries(simulator physics)
target_link_libraries(simulator vehicle)
target_link_libraries(simulator coupling)
target_link_libraries(simulator solver)
target_link_libraries(simulator train)
target_link_libraries(simulator brakepipe)
target_link_libraries(simulator model)
target_link_libraries(simulator TcpConnection)
target_link_libraries(simulator sim-client)
target_link_libraries(simulator profile)
target_link_libraries(simulator device)

target_link_libraries(simulator asound)
target_link_libraries(simulator sound-manager)

set_target_properties(simulator
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(simulator asound)
target_include_directories(simulator PRIVATE ${OPENAL_INCLUDE_DIR})

target_link_libraries(simulator Qt5::Core Qt5::Xml Qt5::Network)
