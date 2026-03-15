add_library(parsing)

target_sources(parsing
    PRIVATE
        src/parse_file_funcs.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES include/parse_file_funcs.h
)
