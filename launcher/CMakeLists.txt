file(GLOB SOURCES
    include/*.h
    src/*.cpp
    src/mainwindow.ui
    resources/launcher.qrc
)
#qt5_add_resources(RESOURCES resources/launcher.qrc)

add_executable(launcher ${SOURCES})

target_link_libraries(launcher Qt::Widgets)

target_link_libraries(launcher Qt::Core)

target_link_libraries(launcher CfgReader)

target_link_libraries(launcher CfgEditor)

target_include_directories(launcher PRIVATE include)

target_include_directories(launcher PRIVATE ../../common-headers)

target_link_libraries(launcher filesystem)

set_target_properties( launcher
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)
