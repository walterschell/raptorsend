cmake_minimum_required(VERSION 2.8.11)
add_executable(sender sender.cpp)
target_link_libraries(sender RaptorQ_Static)
