cmake_minimum_required(VERSION 3.10)
project(CompilerProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
set(SOURCES
    main.cpp
    scanner.cpp
    parser.cpp
)

# Add header files
set(HEADERS
    scanner.h
    parser.h
)

# Create executable
add_executable(compiler ${SOURCES} ${HEADERS})

# Set compiler flags
if(MSVC)
    target_compile_options(compiler PRIVATE /W4)
else()
    target_compile_options(compiler PRIVATE -Wall -Wextra -pedantic)
endif()

# Set include directories
target_include_directories(compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add test executable
add_executable(test_scanner
    test_scanner.cpp
    scanner.cpp
)

# Include directories
target_include_directories(test_scanner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 