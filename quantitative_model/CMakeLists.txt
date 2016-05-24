cmake_minimum_required(VERSION 3.3)
project(quantitative_model)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    Distribution.cpp
    QuantitativeModel.cpp
    QuantitativeModel.h)

add_executable(quantitative_model ${SOURCE_FILES})