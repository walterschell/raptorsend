cmake_minimum_required(VERSION 2.8.11)
add_executable(assembler assembler.cpp)
target_link_libraries(assembler RaptorQ_Static)
