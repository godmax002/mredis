# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.12.0/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.12.0/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/gmax/src/my/mredis

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/gmax/src/my/mredis

# Include any dependencies generated for this target.
include CMakeFiles/mredis.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/mredis.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/mredis.dir/flags.make

CMakeFiles/mredis.dir/redis.c.o: CMakeFiles/mredis.dir/flags.make
CMakeFiles/mredis.dir/redis.c.o: redis.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/gmax/src/my/mredis/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/mredis.dir/redis.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/mredis.dir/redis.c.o   -c /Users/gmax/src/my/mredis/redis.c

CMakeFiles/mredis.dir/redis.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mredis.dir/redis.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/gmax/src/my/mredis/redis.c > CMakeFiles/mredis.dir/redis.c.i

CMakeFiles/mredis.dir/redis.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mredis.dir/redis.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/gmax/src/my/mredis/redis.c -o CMakeFiles/mredis.dir/redis.c.s

# Object files for target mredis
mredis_OBJECTS = \
"CMakeFiles/mredis.dir/redis.c.o"

# External object files for target mredis
mredis_EXTERNAL_OBJECTS =

mredis: CMakeFiles/mredis.dir/redis.c.o
mredis: CMakeFiles/mredis.dir/build.make
mredis: CMakeFiles/mredis.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/gmax/src/my/mredis/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable mredis"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mredis.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/mredis.dir/build: mredis

.PHONY : CMakeFiles/mredis.dir/build

CMakeFiles/mredis.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mredis.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mredis.dir/clean

CMakeFiles/mredis.dir/depend:
	cd /Users/gmax/src/my/mredis && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/gmax/src/my/mredis /Users/gmax/src/my/mredis /Users/gmax/src/my/mredis /Users/gmax/src/my/mredis /Users/gmax/src/my/mredis/CMakeFiles/mredis.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mredis.dir/depend

