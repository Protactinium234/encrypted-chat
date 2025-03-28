# TODO: Actually create this file correctly and compile with it
cmake_minimum_required(VERSION 3.10)
project(encrypted_chat)

set(CMAKE_C_STANDARD 11)

add_executable(chat_server src/chat_server.c)