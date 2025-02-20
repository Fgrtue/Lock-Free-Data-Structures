# 1. Choose the min version of Cmake
cmake_minimum_required(VERSION 3.11)

# 2. Define variable for the project name
set(PROJECT_NAME LockFree)

# Set compiler
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(${PROJECT_NAME} LANGUAGES CXX)

# 3. Set C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# 9. Define library sources and headers
set(HEADERS 
include/hazard-pointers.hpp		  
include/lock-free-spsc-queue.hpp
include/lock-fine-queue.hpp		  
include/lock-free-stack.hpp
include/lock-free-mpmc-bounded-queue.hpp  
include/lock-std-queue.hpp
include/lock-free-mpsc-queue.hpp	  
include/lock-std-stack.hpp
include/lock-free-spmc-queue.hpp
)

set(SOURCES 
src/hazard-pointers.cpp
src/lock-free-spsc-queue.cpp
src/lock-fine-queue.cpp		  
src/lock-free-stack.cpp
src/lock-free-mpmc-bounded-queue.cpp  
src/lock-std-queue.cpp
src/lock-free-mpsc-queue.cpp	  
src/lock-std-stack.cpp
src/lock-free-spmc-queue.cpp
)

# 10. it will be linked with other things
# to form program. We will split our code in
# the actual code and the TEST to go with the code
# the code that is tested is library
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# 11. Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC include)

# 12. Attach threads library 
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
