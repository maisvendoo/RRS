project(RouteReader)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(RouteReader STATIC)

target_sources(RouteReader
    PRIVATE
        src/ObjectsRef.cpp
        src/RouteMap.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/RouteReader/ObjectsRef.h
            include/RouteReader/RouteMap.h
)

target_link_libraries(RouteReader PRIVATE
    Qt6::Core
    Journal
)

install(
    TARGETS RouteReader
    EXPORT RouteReaderTargets
    ARCHIVE DESTINATION sdk/lib
    FILE_SET HEADERS DESTINATION sdk/include
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "RouteReaderConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    EXPORT RouteReaderTargets
    FILE RouteReaderTargets.cmake
    DESTINATION sdk/lib/cmake/RouteReader
)

install(
    FILES
        "RouteReaderConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/RouteReaderConfigVersion.cmake"
    DESTINATION sdk/lib/cmake/RouteReader
)

if(MSVC)
    target_compile_options(RouteReader PRIVATE /W3)
else()
    target_compile_options(RouteReader PRIVATE -Wall -Wextra -Wpedantic)
endif()
