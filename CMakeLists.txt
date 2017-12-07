cmake_minimum_required(VERSION 3.9)
project(threadpool)

file(GLOB SOURCES "*.cpp")

add_library(threadpool STATIC ${SOURCES})