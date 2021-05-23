cmake_minimum_required(VERSION 3.1)

project(armor-find)

find_package(OpenCV REQUIRED)

add_executable(armor_detect_demo
        src/demo.cpp
        src/ArmorDetect.cpp)
target_link_libraries(armor_detect_demo ${OpenCV_LIBS})

add_executable(hist src/hist.cpp)
target_link_libraries(hist ${OpenCV_LIBS})

