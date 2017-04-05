cmake_minimum_required(VERSION 3.7)
project(hw1)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp bitmap.cpp bitmap.h yuv_image.cpp yuv_image.h public_flags.h basic_image.h cmake-build-debug/point.cpp cmake-build-debug/point.h basic_image.cpp kernel.h point.cpp point.h)
add_executable(hw1 ${SOURCE_FILES})