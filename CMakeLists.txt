cmake_minimum_required(VERSION 3.10)
project(encrypted_chat)

# Add your source files
set(SOURCES
    src/chat_server.c
    # Add other source files here
)

# Create the executable
add_executable(chat_server ${SOURCES})