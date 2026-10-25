add_subdirectory (passcar)

if (CLIENT_BUILD)
    add_subdirectory (passcar-iocontroller)
endif()
