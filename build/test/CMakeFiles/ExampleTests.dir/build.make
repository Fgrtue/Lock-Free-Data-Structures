# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kkopnev/Documents/Lock-Free

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kkopnev/Documents/Lock-Free/build

# Include any dependencies generated for this target.
include test/CMakeFiles/ExampleTests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/ExampleTests.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/ExampleTests.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/ExampleTests.dir/flags.make

test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o: test/CMakeFiles/ExampleTests.dir/flags.make
test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o: ../test/ExampleTests.cpp
test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o: test/CMakeFiles/ExampleTests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kkopnev/Documents/Lock-Free/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o"
	cd /home/kkopnev/Documents/Lock-Free/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o -MF CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o.d -o CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o -c /home/kkopnev/Documents/Lock-Free/test/ExampleTests.cpp

test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ExampleTests.dir/ExampleTests.cpp.i"
	cd /home/kkopnev/Documents/Lock-Free/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kkopnev/Documents/Lock-Free/test/ExampleTests.cpp > CMakeFiles/ExampleTests.dir/ExampleTests.cpp.i

test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ExampleTests.dir/ExampleTests.cpp.s"
	cd /home/kkopnev/Documents/Lock-Free/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kkopnev/Documents/Lock-Free/test/ExampleTests.cpp -o CMakeFiles/ExampleTests.dir/ExampleTests.cpp.s

# Object files for target ExampleTests
ExampleTests_OBJECTS = \
"CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o"

# External object files for target ExampleTests
ExampleTests_EXTERNAL_OBJECTS =

test/ExampleTests: test/CMakeFiles/ExampleTests.dir/ExampleTests.cpp.o
test/ExampleTests: test/CMakeFiles/ExampleTests.dir/build.make
test/ExampleTests: lib/libgtest_main.a
test/ExampleTests: libLockFree.a
test/ExampleTests: lib/libgtest.a
test/ExampleTests: test/CMakeFiles/ExampleTests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kkopnev/Documents/Lock-Free/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ExampleTests"
	cd /home/kkopnev/Documents/Lock-Free/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ExampleTests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/ExampleTests.dir/build: test/ExampleTests
.PHONY : test/CMakeFiles/ExampleTests.dir/build

test/CMakeFiles/ExampleTests.dir/clean:
	cd /home/kkopnev/Documents/Lock-Free/build/test && $(CMAKE_COMMAND) -P CMakeFiles/ExampleTests.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/ExampleTests.dir/clean

test/CMakeFiles/ExampleTests.dir/depend:
	cd /home/kkopnev/Documents/Lock-Free/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kkopnev/Documents/Lock-Free /home/kkopnev/Documents/Lock-Free/test /home/kkopnev/Documents/Lock-Free/build /home/kkopnev/Documents/Lock-Free/build/test /home/kkopnev/Documents/Lock-Free/build/test/CMakeFiles/ExampleTests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/ExampleTests.dir/depend

