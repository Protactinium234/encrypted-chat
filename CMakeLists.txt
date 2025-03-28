cmake_minimum_required(VERSION 3.10)
project(encrypted_chat)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cJSON)

# Add your source files
set(SOURCES
    src/chat_server.c
    cjson/cJSON.c
)

# Create the executable
add_executable(chat_server ${SOURCES})

# Enable verbose output
set(CMAKE_VERBOSE_MAKEFILE ON)