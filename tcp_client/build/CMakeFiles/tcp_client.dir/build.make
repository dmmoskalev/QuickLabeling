# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_SOURCE_DIR = /home/padmin/Documents/Projects/quick_labeling/tcp_client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/padmin/Documents/Projects/quick_labeling/tcp_client/build

# Include any dependencies generated for this target.
include CMakeFiles/tcp_client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/tcp_client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tcp_client.dir/flags.make

CMakeFiles/tcp_client.dir/tcp_client.cpp.o: CMakeFiles/tcp_client.dir/flags.make
CMakeFiles/tcp_client.dir/tcp_client.cpp.o: ../tcp_client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/padmin/Documents/Projects/quick_labeling/tcp_client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tcp_client.dir/tcp_client.cpp.o"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tcp_client.dir/tcp_client.cpp.o -c /home/padmin/Documents/Projects/quick_labeling/tcp_client/tcp_client.cpp

CMakeFiles/tcp_client.dir/tcp_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tcp_client.dir/tcp_client.cpp.i"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/padmin/Documents/Projects/quick_labeling/tcp_client/tcp_client.cpp > CMakeFiles/tcp_client.dir/tcp_client.cpp.i

CMakeFiles/tcp_client.dir/tcp_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tcp_client.dir/tcp_client.cpp.s"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/padmin/Documents/Projects/quick_labeling/tcp_client/tcp_client.cpp -o CMakeFiles/tcp_client.dir/tcp_client.cpp.s

# Object files for target tcp_client
tcp_client_OBJECTS = \
"CMakeFiles/tcp_client.dir/tcp_client.cpp.o"

# External object files for target tcp_client
tcp_client_EXTERNAL_OBJECTS =

tcp_client: CMakeFiles/tcp_client.dir/tcp_client.cpp.o
tcp_client: CMakeFiles/tcp_client.dir/build.make
tcp_client: /lib/aarch64-linux-gnu/libpthread.so.0
tcp_client: CMakeFiles/tcp_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/padmin/Documents/Projects/quick_labeling/tcp_client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tcp_client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tcp_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tcp_client.dir/build: tcp_client

.PHONY : CMakeFiles/tcp_client.dir/build

CMakeFiles/tcp_client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tcp_client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tcp_client.dir/clean

CMakeFiles/tcp_client.dir/depend:
	cd /home/padmin/Documents/Projects/quick_labeling/tcp_client/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/padmin/Documents/Projects/quick_labeling/tcp_client /home/padmin/Documents/Projects/quick_labeling/tcp_client /home/padmin/Documents/Projects/quick_labeling/tcp_client/build /home/padmin/Documents/Projects/quick_labeling/tcp_client/build /home/padmin/Documents/Projects/quick_labeling/tcp_client/build/CMakeFiles/tcp_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tcp_client.dir/depend

