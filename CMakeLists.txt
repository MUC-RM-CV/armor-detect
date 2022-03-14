cmake_minimum_required(VERSION 3.1)

project(armor-find)

find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV Version: ${OpenCV_VERSION}")

# Armor-Detect Library

add_library(armor_detect OBJECT src/ArmorDetect.cpp)
target_include_directories(armor_detect PUBLIC src)
target_link_libraries(armor_detect ${OpenCV_LIBS})

# Armor-Detect Demo

add_executable(demo src/demo.cpp)
target_link_libraries(demo armor_detect)

# Hist Demo

add_executable(hist src/hist.cpp)
target_link_libraries(hist ${OpenCV_LIBS})
