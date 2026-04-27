cmake_minimum_required(VERSION 3.10)
project(BasicClient)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(Client Client.cpp)

# Platform-specific linking
if(WIN32)
    # Link Winsock2 on Windows
    target_link_libraries(Client PRIVATE ws2_32)
else()
    # Link pthread on Unix/Linux systems
    find_package(Threads REQUIRED)
    target_link_libraries(Client PRIVATE Threads::Threads)
endif()

# Enable compiler warnings
if(MSVC)
    target_compile_options(Client PRIVATE /W4)
else()
    target_compile_options(Client PRIVATE -Wall -Wextra -Wpedantic)
endif()
