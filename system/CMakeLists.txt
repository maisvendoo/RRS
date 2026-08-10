set(TARGET system)

project(${TARGET})

add_definitions(-DSYSTEM_LIB)

add_library(${TARGET} STATIC)

target_sources(${TARGET}
    PRIVATE
        src/crash-handler.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
        include
    FILES
        include/crash-handler.h
)

if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W3)
else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()
