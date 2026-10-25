set(TESTS
    normalize_mouse_coordinates
)

foreach(name IN LISTS TESTS)
    add_executable(test_${name})
    target_sources(test_${name}
        PRIVATE
            test_${name}.cpp

        PRIVATE
            FILE_SET HEADERS
            BASE_DIRS
                ../include
            FILES
                ../include/editor/editor_math.h
    )
    target_link_libraries(test_${name} PRIVATE
        vsg::vsg
    )
    add_test(NAME test_${name} COMMAND test_${})
endforeach()
