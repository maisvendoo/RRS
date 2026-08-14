# cmake/BuildConfiguration.cmake
# Дополнительные настройки компиляции

if(DEFINED BUILD_CONFIGURATION_INCLUDED)
    return()
endif()
set(BUILD_CONFIGURATION_INCLUDED TRUE)

message(STATUS "========================================")
message(STATUS "Configuring build settings...")
message(STATUS "========================================")

# ============================================
# 1. ОТЛАДОЧНЫЕ СИМВОЛЫ ДЛЯ RELWITHDEBINFO
# ============================================

# Добавляем отладочные символы для RelWithDebInfo
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        # Добавляем отладочную информацию
        add_compile_options(-g)
        # Сохраняем frame pointer для backtrace
        add_compile_options(-fno-omit-frame-pointer)
        
        # Для Unix добавляем -rdynamic для backtrace
        if(UNIX AND NOT MINGW)
            add_compile_options(-rdynamic)
            add_link_options(-rdynamic)
        endif()
        
        message(STATUS "  - RelWithDebInfo: debug symbols enabled")
    endif()
endif()

# ============================================
# 2. ADDRESS SANITIZER (ASan) ДЛЯ DEBUG
# ============================================

option(ENABLE_ASAN "Enable Address Sanitizer for Debug builds" OFF)

if(ENABLE_ASAN)
    # ASan доступен только в Unix-системах
    if(UNIX AND NOT MINGW)
        # Проверяем, что сборка в Debug
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_compile_options(
                -fsanitize=address
                -fno-omit-frame-pointer
                -g
            )
            add_link_options(
                -fsanitize=address
            )
            
            # Для корректной работы ASan в Linux
            if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
                add_link_options(-Wl,--no-as-needed)
            endif()
            
            message(STATUS "  - ASan: enabled for Debug build")
        else()
            message(WARNING "  - ASan: only works with Debug build type (current: ${CMAKE_BUILD_TYPE})")
            message(WARNING "  - ASan: use -DCMAKE_BUILD_TYPE=Debug to enable")
        endif()
    else()
        message(WARNING "  - ASan: only supported on Unix (not MinGW/Windows)")
    endif()
endif()

# ============================================
# 3. ДОПОЛНИТЕЛЬНЫЕ ОПЦИИ
# ============================================

# Можно добавить свои флаги через переменную окружения
if(DEFINED ENV{USER_COMPILE_FLAGS})
    message(STATUS "  - User compile flags: $ENV{USER_COMPILE_FLAGS}")
    add_compile_options($ENV{USER_COMPILE_FLAGS})
endif()

if(DEFINED ENV{USER_LINK_FLAGS})
    message(STATUS "  - User link flags: $ENV{USER_LINK_FLAGS}")
    add_link_options($ENV{USER_LINK_FLAGS})
endif()

message(STATUS "========================================")