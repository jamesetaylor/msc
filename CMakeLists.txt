cmake_minimum_required(VERSION 3.17)
project(msc)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.71 REQUIRED COMPONENTS
        unit_test_framework
        )

include_directories(${Boost_INCLUDE_DIRS})

add_executable(rc5plus rc5plus.cpp)
add_executable(rc5plus2 rc5plus2.cpp)
add_executable(lfsr lfsr.cpp)
add_executable(geffe geffe.cpp BitStreamTests.hpp)
add_executable(rc4 rc4.cpp)
