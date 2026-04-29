# Устанавливаем переменную TARGET равной имени проекта,
# чтобы потом везде подставлять
set(TARGET dmd2gltf)

# Задаем имя проекта
project(${TARGET})

# Путь, куда надо сложить собранное
set(EXECUTABLE_OUTPUT_PATH "../../../bin")

find_package(Vulkan REQUIRED)

find_package(KTX QUIET)
if(NOT KTX_FOUND)
    find_package(ktx QUIET)
endif()
if(NOT ktx_FOUND)
    find_package(Ktx QUIET)
endif()
if(NOT KTX_FOUND AND NOT ktx_FOUND AND NOT Ktx_FOUND)
    message(FATAL_ERROR "KTX library not found. Install libktx v4+ with BASIS & Vulkan support.")
endif()

if(TARGET KTX::ktx)
    set(KTX_TARGET KTX::ktx)
elseif(TARGET ktx::ktx)
    set(KTX_TARGET ktx::ktx)
elseif(TARGET Ktx::ktx)
    set(KTX_TARGET Ktx::ktx)
else()
    message(FATAL_ERROR "Found KTX but could not resolve imported target name.")
endif()

# Пути к заголовкам и сорцам
file(GLOB INCLUDES "./include/*.h" "./include/*.hpp")
file(GLOB SOURCES "./src/*.cpp")

# Компиляция приложения
add_executable(${TARGET} ${INCLUDES} ${SOURCES})

# Указываем пути поиска заголовков
target_include_directories(${TARGET} PRIVATE ./include)

# Компонуемся с библиотеками
target_link_libraries (${TARGET} filesystem)
target_link_libraries (${TARGET} ${KTX_TARGET})
target_link_libraries (${TARGET} Vulkan::Vulkan)
