# TODO: Actually create this file correctly and compile with it
cmake_minimum_required(VERSION 3.10)
project(encrypted_chat)

set(CMAKE_C_STANDARD 11)

# Add cJSON as a subdirectory
add_subdirectory(cJSON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cJSON)

# Add your source files
set(SOURCES
    src/chat_server.c
)

# Create the executable
add_executable(chat_server ${SOURCES})

# Link cJSON library to your executable
target_link_libraries(chat_server cjson)