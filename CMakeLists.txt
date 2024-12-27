# 1. Choose the min version of Cmake
cmake_minimum_required(VERSION 3.8)

# 2. Define variable for the project name
set(PROJECT_NAME LockFree)

project(${PROJECT_NAME} C CXX)

# 3. Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# 4. Enables position inpdendent code
# is compiled in a way that allows the generated machine code
# to be loaded at any memory address
# essential for creating shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 5. Allows have unit testing
enable_testing()

add_subdirectory(googletest)

# 6. Add ThreadSanitizer flags if enabled
if(ENABLE_TSAN)
    set(CMAKE_C_FLAGS "-fsanitize=thread -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "-fsanitize=thread -fno-omit-frame-pointer -g")
endif()

# 7. Define library sources
set(HEADERS include/lock-std-queue.hpp)
set(SOURCES src/lock-std-queue.cpp)

# 8. it will be linked with other things
# to form program. We will split our code in
# the actual code and the TEST to go with the code
# the code that is tested is library
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# 9. Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC include)

# 10. Add the test subdirectory
add_subdirectory(test)

