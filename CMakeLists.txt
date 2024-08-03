cmake_minimum_required (VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (RRS)

add_subdirectory (CfgReader)
add_subdirectory (CfgEditor)
add_subdirectory (filesystem)
add_subdirectory (libJournal)

