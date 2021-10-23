file(GLOB_RECURSE SOURCES
    include/*.h
    src/*.cpp
)



find_package(Qt5 COMPONENTS Quick Core REQUIRED)
qt5_add_resources(RESOURCES launcher.qrc)
add_executable(launcher2 ${SOURCES} ${RESOURCES})

target_include_directories(launcher2 PRIVATE include)

target_link_libraries(launcher2 filesystem)

target_link_libraries(launcher2 CfgReader)

set_target_properties(launcher2
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

qt5_use_modules(launcher2 Quick Core)
