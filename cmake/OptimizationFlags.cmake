# cmake/OptimizationFlags.cmake
# Устанавливает флаги оптимизации для разных конфигураций

# Базовые флаги для всех конфигураций
target_compile_options(optflags INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall
        -Wextra
        -Wpedantic
        -fno-math-errno
        -fno-trapping-math
    >
)

# Флаги только для релизных конфигураций
target_compile_options(optflags INTERFACE
    $<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -O3
            -march=native          # см. предупреждение ниже
            -funroll-loops
            -ffinite-math-only     # безопасно для симуляции
            -fno-signed-zeros
        >
    >
)

# Отладочные конфигурации: безопасные флаги
target_compile_options(optflags INTERFACE
    $<$<CONFIG:Debug>:
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -Og                    # оптимизация для отладки
            -g3
            -fno-omit-frame-pointer
        >
    >
)

# ================================================
# НОВЫЕ ФЛАГИ ДЛЯ CRASH-HANDLER
# ================================================

# Релиз с отладочными символами (RelWithDebInfo)
# Это позволяет получать осмысленные стектрейсы в продакшене
target_compile_options(optflags INTERFACE
    $<$<CONFIG:RelWithDebInfo>:
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -g                    # отладочные символы
            -fno-omit-frame-pointer
        >
    >
)

# Для всех Unix-конфигураций добавляем -rdynamic для backtrace
# Это нужно для получения имен функций в стектрейсе
if(UNIX)
    target_compile_options(optflags INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            $<$<CONFIG:Debug,RelWithDebInfo>:
                -rdynamic
            >
        >
    )
    
    # Добавляем флаги линковщика
    target_link_options(optflags INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            $<$<CONFIG:Debug,RelWithDebInfo>:
                -rdynamic
                -Wl,--build-id=sha1
            >
        >
    )
endif()

# Предупреждение о -march=native при кросс-компиляции
if(CMAKE_CROSSCOMPILING AND NOT DEFINED ALLOW_NATIVE_ARCH)
    message(WARNING " -march=native отключён при кросс-компиляции. "
                    "Установите -DALLOW_NATIVE_ARCH=ON для принудительного включения.")
    target_compile_options(optflags INTERFACE
        $<$<CONFIG:Release,RelWithDebInfo>:
            $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
                -march=haswell     # безопасный минимум для современных CPU
            >
        >
    )
endif()