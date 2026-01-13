set(TARGET scenario-manager)

project(${TARGET})

set(CMAKE_AUTOMOC ON)

set (LIBRARY_OUTPUT_PATH "../../../lib")

set (CMAKE_SHARED_LIBRARY_PREFIX "")

add_definitions(-DSCNMGR_LIB)

find_package(Qt6 COMPONENTS Core REQUIRED)

if(WIN32)
    find_package(lua CONFIG REQUIRED)
else()
    find_package(Lua 5.4 REQUIRED)
endif()

find_package(sol2 REQUIRED)

file (GLOB INCLUDES "./include/*.h")
file (GLOB SOURCES "./src/*.cpp")

add_library (${TARGET} SHARED ${SOURCES} ${INCLUDES})

target_include_directories (${TARGET} PRIVATE ./include/)
target_include_directories (${TARGET} PRIVATE ../../libJournal/include)
target_include_directories (${TARGET} PRIVATE ../../common-headers)
target_include_directories (${TARGET} PRIVATE ../topology/include)

target_link_libraries(${TARGET} Qt6::Core)

if(WIN32)
    target_link_libraries(${TARGET} lua::lua)
else()
    target_link_libraries(${TARGET} ${LUA_LIBRARIES})
endif()

target_link_libraries(${TARGET} sol2)

target_link_libraries (${TARGET} Journal)
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} device)
