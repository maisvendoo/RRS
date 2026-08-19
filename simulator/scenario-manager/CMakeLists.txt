set(TARGET scenario-manager)

project(${TARGET})

set(CMAKE_AUTOMOC ON)

set (LIBRARY_OUTPUT_PATH "../../../lib")

set (CMAKE_SHARED_LIBRARY_PREFIX "")

add_definitions(-DSCNMGR_LIB)

find_package(Qt6 COMPONENTS Core REQUIRED)

if(WIN32 OR APPLE)
    find_package(lua CONFIG REQUIRED)
else()
    find_package(Lua 5.4 REQUIRED)
endif()

find_package(sol2 QUIET)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../common-headers)

target_link_libraries(${TARGET} Qt6::Core)

if(WIN32 OR APPLE)
    # On macOS: ensure Lua headers from lua-cmake are found before system Lua 5.5
    if(APPLE)
        get_target_property(_LUA_INC lua::lua INTERFACE_INCLUDE_DIRECTORIES)
        target_include_directories(${TARGET} BEFORE PRIVATE ${_LUA_INC})
    endif()
    target_link_libraries(${TARGET} lua::lua)
else()
    target_link_libraries(${TARGET} ${LUA_LIBRARIES})
    target_include_directories(${TARGET} PRIVATE ${LUA_INCLUDE_DIR})
endif()

if(TARGET sol2::sol2)
    target_link_libraries(${TARGET} sol2::sol2)
elseif(TARGET sol2)
    target_link_libraries(${TARGET} sol2)
else()
    message(FATAL_ERROR "sol2 library not found")
endif()

target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} device)
target_link_libraries (${TARGET} topology)
