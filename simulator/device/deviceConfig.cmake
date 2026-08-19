include(CMakeFindDependencyMacro)
# find_dependency(xx 2.0)
find_dependency(solver)
include(${CMAKE_CURRENT_LIST_DIR}/deviceTargets.cmake)