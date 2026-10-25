add_subdirectory (freightcar)

if (CLIENT_BUILD)
    add_subdirectory (freightcar-iocontroller)
endif()