cmake_minimum_required(VERSION 3.16.0)
project(CSCI425 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB nfamatch_sources nfamatch/**.cpp)
add_executable(NFAMATCH nfamatch/main.cpp ${nfamatch_sources})
