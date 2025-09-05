cmake_minimum_required(VERSION 3.11.0)
project(cregex VERSION 1.0.0)
add_executable(cregex src/example.c)
target_include_directories(cregex PRIVATE ./include)