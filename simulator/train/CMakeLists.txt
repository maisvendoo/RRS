file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(train ${SOURCES})

target_include_directories(train PUBLIC include)

target_link_libraries(train CfgReader)
target_link_libraries(train filesystem)
target_link_libraries(train Journal)
target_link_libraries(train physics)
target_link_libraries(train vehicle)
target_link_libraries(train coupling)
target_link_libraries(train solver)
target_link_libraries(train brakepipe)
target_link_libraries(train profile)
target_link_libraries(train device)

target_link_libraries(train asound)
target_link_libraries(train sound-manager)

target_include_directories(train PRIVATE ../../common-headers)

target_include_directories(train PRIVATE ${OPENAL_INCLUDE_DIR})

set_target_properties( train
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(train Qt5::Core Qt5::Xml)
