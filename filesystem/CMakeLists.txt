file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_library(filesystem ${SOURCES})

target_include_directories(filesystem PUBLIC include)

#set_target_properties(filesystem PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

set_target_properties( filesystem
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(filesystem Qt5::Core)
