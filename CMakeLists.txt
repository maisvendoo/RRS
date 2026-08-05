cmake_minimum_required (VERSION 3.29)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (RRS VERSION 1.9.3)

# RPATH configuration for finding shared libraries at runtime
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_MACOSX_RPATH ON)
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# No lib prefix in all shared libraries
set(CMAKE_SHARED_LIBRARY_PREFIX "")

option (CLIENT_BUILD "Build for client usage" ON)

add_subdirectory (CfgReader)
add_subdirectory (CfgEditor)
add_subdirectory (filesystem)

add_subdirectory (libJournal)
add_subdirectory (network)
add_subdirectory (parsing)
add_subdirectory (core)

add_subdirectory (simulator)

add_subdirectory (addons/passcar)
add_subdirectory (addons/freightcar)
add_subdirectory (addons/vl60)

if (CLIENT_BUILD)
    add_subdirectory (graphics)
    add_subdirectory (sound-manager)
    add_subdirectory (viewer)

    add_subdirectory (route_editor)
    #add_subdirectory (route_editor2)

    add_subdirectory (launcher)

    add_subdirectory (tools/dmd2gltf)
    add_subdirectory (tools/profconv)
    add_subdirectory (tools/pathconv)
    add_subdirectory (tools/offset-parallel-gen)
    add_subdirectory (tools/offset-spline-gen)
    add_subdirectory (tools/topologycheck)
    add_subdirectory (tools/routeconv)
    add_subdirectory (tools/route-map)
    add_subdirectory (tools/route-transform)
    add_subdirectory (tools/texture-compressor)
endif()

include(CMakePackageConfigHelpers)

# ПРАВИЛЬНЫЙ ПОРЯДОК КОМПОНЕНТОВ (с учетом зависимостей)
# Сначала независимые, потом зависимые
set(RRS_SDK_COMPONENTS
    # Базовые (не имеют зависимостей внутри SDK)
    core
    filesystem
    Journal
    physics

    # Промежуточные
    solver
    CfgReader

    # Высокоуровневые (зависят от предыдущих)
    device      # зависит от filesystem, Journal, physics
    display
    io-controller
    vehicle     # зависит от CfgReader, device, Journal, physics
)

# Создаем шаблон (содержимое одинаково для всех платформ)
set(CONFIG_IN_CONTENT "
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

# Автоматически определяем RRS_DIR (работает на Windows/Linux/macOS)
get_filename_component(RRS_CMAKE_DIR \"\${CMAKE_CURRENT_LIST_DIR}\" ABSOLUTE)
get_filename_component(RRS_SDK_LIB_DIR \"\${RRS_CMAKE_DIR}\" DIRECTORY)
get_filename_component(RRS_SDK_DIR \"\${RRS_SDK_LIB_DIR}\" DIRECTORY)
get_filename_component(RRS_DIR \"\${RRS_SDK_DIR}\" DIRECTORY)

list(APPEND CMAKE_PREFIX_PATH \"\${RRS_CMAKE_DIR}\")

set(RRS_INCLUDE_DIR \"\${RRS_SDK_DIR}/include\")
set(RRS_LIBRARY_DIR \"\${RRS_SDK_DIR}/lib\")

foreach(comp ${RRS_SDK_COMPONENTS})
    if(EXISTS \"\${RRS_CMAKE_DIR}/\${comp}/\${comp}Config.cmake\")
        find_dependency(\${comp})
    endif()
endforeach()
")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/RRSConfig.cmake.in "${CONFIG_IN_CONTENT}")

configure_package_config_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RRSConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RRSConfig.cmake
    INSTALL_DESTINATION sdk/lib/cmake
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RRSConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/RRSConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/RRSConfigVersion.cmake
    DESTINATION sdk/lib/cmake
)
