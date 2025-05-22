cmake_minimum_required(VERSION 3.30)

project(funcc)

set(CMAKE_CXX_STANDARD 17)

add_executable(funcc)
file(GLOB_RECURSE funcc_sources src/*.cc)
target_sources(funcc PRIVATE ${funcc_sources})
