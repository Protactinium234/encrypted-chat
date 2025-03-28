# Use an official Ubuntu base image
FROM ubuntu:20.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && apt-get install -y cmake build-essential libssl-dev nginx libwebsockets-dev libcjson-dev && rm -rf /var/lib/apt/lists/*

# Create and set the working directory
WORKDIR /usr/src/app

# Copy the project files to the working directory
COPY . .

# Create a build directory
RUN mkdir -p build

# Change to the build directory
WORKDIR /usr/src/app/build

# Run CMake to configure the project
RUN cmake ..

# Build the project
RUN make

# Copy static files to Nginx's default directory
RUN mkdir -p /var/www/html/static
COPY static /var/www/html/static

# Copy Nginx configuration file
COPY nginx.conf /etc/nginx/nginx.conf

# Expose the port for Nginx
EXPOSE 80

# Expose the port for the backend server
EXPOSE 8080

# Command to run both Nginx and the backend server
CMD service nginx start && ./encrypted_chat