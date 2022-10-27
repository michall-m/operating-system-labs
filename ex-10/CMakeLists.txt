cmake_minimum_required(VERSION 3.15)
project(zad01 C)

set(CMAKE_C_STANDARD 11)
add_executable(zad01 server.c client.c board.h)