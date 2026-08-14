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
# 1. POSITION INDEPENDENT CODE (-fPIC)
# ============================================

# Включаем -fPIC для всех целей на Unix (нужно для shared libraries)
if(UNIX)
    # Для всех целей (включая статические библиотеки)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    
    # Дополнительно для GCC/Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-fPIC)
        message(STATUS "  - -fPIC enabled for all targets")
    endif()
endif()

# ============================================
# 1. УРОВЕНЬ ОПТИМИЗАЦИИ ДЛЯ RELEASE
# ============================================

# Опция для выбора уровня оптимизации в Release
# Возможные значения: O0, O1, O2, O3, Os, Ofast
set(RELEASE_OPTIMIZATION_LEVEL "O3" CACHE STRING 
    "Optimization level for Release builds (O0, O1, O2, O3, Os, Ofast)")

# Проверка корректности значения
if(NOT RELEASE_OPTIMIZATION_LEVEL MATCHES "^(O0|O1|O2|O3|Os|Ofast)$")
    message(WARNING "  - Invalid RELEASE_OPTIMIZATION_LEVEL: ${RELEASE_OPTIMIZATION_LEVEL}")
    message(WARNING "  - Using default: O3")
    set(RELEASE_OPTIMIZATION_LEVEL "O3" CACHE STRING 
        "Optimization level for Release builds (O0, O1, O2, O3, Os, Ofast)" FORCE)
endif()

# Применяем уровень оптимизации для Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-${RELEASE_OPTIMIZATION_LEVEL})
        message(STATUS "  - Release optimization level: -${RELEASE_OPTIMIZATION_LEVEL}")
        
        # Дополнительные флаги для высоких уровней оптимизации
        if(RELEASE_OPTIMIZATION_LEVEL STREQUAL "O3" OR RELEASE_OPTIMIZATION_LEVEL STREQUAL "Ofast")
            add_compile_options(
                -funroll-loops
                -ffinite-math-only
                -fno-signed-zeros
            )
            message(STATUS "  - Additional optimizations enabled (unroll-loops, fast-math)")
        endif()
        
        # Для Ofast добавляем еще больше агрессивных оптимизаций
        if(RELEASE_OPTIMIZATION_LEVEL STREQUAL "Ofast")
            add_compile_options(
                -fno-math-errno
                -fno-trapping-math
            )
            message(STATUS "  - Ofast: aggressive math optimizations enabled")
        endif()
    endif()
endif()

# ============================================
# 2. ОТЛАДОЧНЫЕ СИМВОЛЫ ДЛЯ RELWITHDEBINFO
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
# 3. ADDRESS SANITIZER (ASan) ДЛЯ DEBUG
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
# 4. ДОПОЛНИТЕЛЬНЫЕ ОПЦИИ
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