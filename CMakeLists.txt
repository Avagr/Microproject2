cmake_minimum_required(VERSION 3.17)
project(BarberShop)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads)
add_executable(BarberShop main.cpp)
target_link_libraries(BarberShop ${CMAKE_THREAD_LIBS_INIT})