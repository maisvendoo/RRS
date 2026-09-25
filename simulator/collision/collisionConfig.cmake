include(CMakeFindDependencyMacro)
# Jolt собирается вместе с проектом и линкуется внутрь collision,
# отдельный find_dependency(Jolt) не требуется
include(${CMAKE_CURRENT_LIST_DIR}/collisionTargets.cmake)
