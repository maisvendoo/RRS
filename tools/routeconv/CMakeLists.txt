file(GLOB SOURCES
    include/*.h
    src/*.cpp
    forms/mainwindow.ui
    resources/routeconv.qrc
)

add_executable(routeconv ${SOURCES})

target_link_libraries(routeconv filesystem)

target_link_libraries(routeconv CfgEditor)

target_include_directories(routeconv PRIVATE ../../common-headers)

target_include_directories(routeconv PRIVATE include)



set_target_properties( routeconv
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}../../bin"
)

target_link_libraries(routeconv Qt5::Core Qt::Widgets)
