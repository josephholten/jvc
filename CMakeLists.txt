cmake_minimum_required(VERSION 3.15.0)
project(jvc LANGUAGES C CXX)

add_subdirectory(docopt.cpp)

add_executable(jvc jvc.cpp)
target_link_libraries(jvc PRIVATE docopt)
