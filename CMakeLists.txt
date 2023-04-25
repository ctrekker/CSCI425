cmake_minimum_required(VERSION 3.16.0)
project(CSCI425 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB common_sources common/**.cpp)
add_library(COMMON ${common_sources})

file(GLOB nfamatch_sources match/**.cpp)
add_executable(NFAMATCH match/main.cpp ${nfamatch_sources})
target_link_libraries(NFAMATCH COMMON)
target_include_directories(NFAMATCH PRIVATE .)

file(GLOB cfg_sources cfg/**.cpp)
add_executable(CFG cfg/main.cpp ${cfg_sources})
target_link_libraries(CFG COMMON)
target_include_directories(CFG PRIVATE .)

file(GLOB luther_sources luther/**.cpp)
add_executable(LUTHER luther/main.cpp ${luther_sources})
target_link_libraries(LUTHER COMMON)
target_include_directories(LUTHER PRIVATE .)

file (GLOB lga_sources lga/**.cpp)
add_executable(LGA lga/main.cpp ${lga_sources})
target_link_libraries(LGA COMMON)
target_include_directories(LGA PRIVATE .)

file (GLOB wreck_sources wreck/**.cpp)
add_executable(WRECK wreck/main.cpp ${wreck_sources})
target_link_libraries(WRECK COMMON)
target_include_directories(WRECK PRIVATE .)
