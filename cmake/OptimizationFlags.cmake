# cmake/OptimizationFlags.cmake
# Устанавливает флаги оптимизации для разных конфигураций

# Базовые флаги для всех конфигураций
target_compile_options(optflags INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:
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
        $<$<CXX_COMPILER_ID:GNU,Clang>:
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
        $<$<CXX_COMPILER_ID:GNU,Clang>:
            -Og                    # оптимизация для отладки
            -g3
            -fno-omit-frame-pointer
        >
    >
)

# Предупреждение о -march=native при кросс-компиляции
if(CMAKE_CROSSCOMPILING AND NOT DEFINED ALLOW_NATIVE_ARCH)
    message(WARNING " -march=native отключён при кросс-компиляции. "
                    "Установите -DALLOW_NATIVE_ARCH=ON для принудительного включения.")
    target_compile_options(optflags INTERFACE
        $<$<CONFIG:Release,RelWithDebInfo>:
            $<$<CXX_COMPILER_ID:GNU,Clang>:
                -march=haswell     # безопасный минимум для современных CPU
            >
        >
    )
endif()