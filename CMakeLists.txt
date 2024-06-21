cmake_minimum_required(VERSION 3.10)
project(encrypted_chat)

set(CMAKE_C_STANDARD 11)

add_executable(encrypted_chat src/server.c)