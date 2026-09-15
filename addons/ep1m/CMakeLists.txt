# ЭП1М - пассажирский электровоз переменного тока (davydov-dt/ep1m)
add_subdirectory (ep1m)
add_subdirectory (ep1m-autopilot)

if (CLIENT_BUILD)
    add_subdirectory (ep1m-iocontroller)
    add_subdirectory (msud-display)
    add_subdirectory (club-u-display)
endif()
