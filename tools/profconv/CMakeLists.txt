file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

add_executable(profconv ${SOURCES})

target_include_directories(profconv PRIVATE include)

set_target_properties( profconv
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(profconv Qt5::Core)
