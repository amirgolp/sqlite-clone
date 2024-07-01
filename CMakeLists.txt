cmake_minimum_required(VERSION 3.10)
project(sqlite)

set(CMAKE_CXX_STANDARD 17)

# Add all source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(sqlite ${SOURCES})

# Include directories
target_include_directories(sqlite PRIVATE include)

# Link against pthread
target_link_libraries(sqlite PRIVATE pthread)
