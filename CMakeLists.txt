# 1. Choose the min version of Cmake
cmake_minimum_required(VERSION 3.8)

# 2. Define variable for the project name
set(PROJECT_NAME LockFree)

# Set compiler
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(${PROJECT_NAME} CXX)

# 3. Set C and C++ standards
set(CMAKE_CXX_STANDARD 17)

# 4. Enables position inpdendent code
# is compiled in a way that allows the generated machine code
# to be loaded at any memory address
# essential for creating shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 5. Allows have unit testing
enable_testing()

# 6. Add ThreadSanitizer flags if enabled
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -fsanitize=thread -fno-omit-frame-pointer -g -pthread")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif()

# 7. Add the google test subdirectory
#   and test subdirectory

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

FetchContent_MakeAvailable(googletest)

add_subdirectory(test)

# 8. Add the google bencmark subdirectory
#   and benchmark subdirectory
 add_subdirectory(benchmark) 
 add_subdirectory(benchmarks) 

# 9. Define library sources
file(GLOB_RECURSE HEADERS "include/*.hpp")
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 10. it will be linked with other things
# to form program. We will split our code in
# the actual code and the TEST to go with the code
# the code that is tested is library
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# 11. Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC include)

# 12. Attach threads library 
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads rt)
