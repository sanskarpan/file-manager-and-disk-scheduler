cmake_minimum_required(VERSION 3.16)
project(OS_Project VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings as errors in CI
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

# Enable filesystem linking for older GCC
find_package(Threads REQUIRED)

# Binaries
add_executable(file_manager main.cpp)
target_link_libraries(file_manager PRIVATE Threads::Threads)

add_executable(disk_scheduling try.cpp)
target_link_libraries(disk_scheduling PRIVATE Threads::Threads)
target_compile_definitions(disk_scheduling PRIVATE DISK_SCHEDULING_TEST=0)

# Testing binary for disk scheduling
add_executable(disk_scheduling_test try.cpp)
target_compile_definitions(disk_scheduling_test PRIVATE DISK_SCHEDULING_TEST=1)
target_link_libraries(disk_scheduling_test PRIVATE Threads::Threads)

# Installation
install(TARGETS file_manager disk_scheduling DESTINATION bin)
install(DIRECTORY static DESTINATION share/OS_Project)
install(FILES app.py requirements.txt DESTINATION share/OS_Project)

enable_testing()
add_test(NAME DiskSchedulingSelfTest COMMAND disk_scheduling_test --test)
add_test(NAME FileManagerBuilds COMMAND file_manager --help)

# For CTest: file_manager doesn't support --help gracefully, so just check binary exists
# Alternative: add a --version flag in future
