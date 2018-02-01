cmake_minimum_required(VERSION 3.9)
project(small-tl)

file(GLOB SOURCES "*.cpp")
file(GLOB THREADING "threading/*.cpp")
file(GLOB UTF "utf/*.cpp")

add_library(small-tl STATIC ${SOURCES} ${THREADING} ${UTF})