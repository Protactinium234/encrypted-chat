cmake_minimum_required(VERSION 3.10)
project(encrypted_chat)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
include_directories(/usr/include)

# Add your source files
set(SOURCES
    src/chat_server.c
    # Add other source files here
)

# Create the executable
add_executable(chat_server ${SOURCES})

# Link cJSON library to your executable
target_link_libraries(chat_server ${CJSON_LIBRARIES})

# Enable verbose output
set(CMAKE_VERBOSE_MAKEFILE ON)