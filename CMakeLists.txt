cmake_minimum_required(VERSION 3.12)
project(MyProject VERSION 1.0.0)

#source files
file(GLOB SOURCES "src/*.cpp")
add_executable(main ${SOURCES})

include_directories(include)
