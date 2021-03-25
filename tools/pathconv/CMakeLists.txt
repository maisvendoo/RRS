file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_executable(pathconv ${SOURCES})

target_include_directories(pathconv PRIVATE include)

set_target_properties( pathconv
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(pathconv Qt5::Core)
