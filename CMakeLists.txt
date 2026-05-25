cmake_minimum_required(VERSION 4.0)
project(utmn)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(utmn
    src/csv.cpp
    src/marketplace.cpp
    src/main.cpp
)