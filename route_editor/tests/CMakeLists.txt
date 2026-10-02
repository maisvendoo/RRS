add_executable(tests)

target_sources(tests
    PRIVATE
        tests.cpp
)

add_test(NAME tests COMMAND tests)
