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
CMAKE_SOURCE_DIR = /home/padmin/Documents/Projects/quick_labeling/ql

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/padmin/Documents/Projects/quick_labeling/ql/build

# Include any dependencies generated for this target.
include CMakeFiles/ql.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ql.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ql.dir/flags.make

CMakeFiles/ql.dir/ql.cpp.o: CMakeFiles/ql.dir/flags.make
CMakeFiles/ql.dir/ql.cpp.o: ../ql.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/padmin/Documents/Projects/quick_labeling/ql/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ql.dir/ql.cpp.o"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ql.dir/ql.cpp.o -c /home/padmin/Documents/Projects/quick_labeling/ql/ql.cpp

CMakeFiles/ql.dir/ql.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ql.dir/ql.cpp.i"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/padmin/Documents/Projects/quick_labeling/ql/ql.cpp > CMakeFiles/ql.dir/ql.cpp.i

CMakeFiles/ql.dir/ql.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ql.dir/ql.cpp.s"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/padmin/Documents/Projects/quick_labeling/ql/ql.cpp -o CMakeFiles/ql.dir/ql.cpp.s

# Object files for target ql
ql_OBJECTS = \
"CMakeFiles/ql.dir/ql.cpp.o"

# External object files for target ql
ql_EXTERNAL_OBJECTS =

ql: CMakeFiles/ql.dir/ql.cpp.o
ql: CMakeFiles/ql.dir/build.make
ql: /usr/local/lib/aarch64-linux-gnu/libopencv_core.so
ql: /usr/local/lib/aarch64-linux-gnu/libopencv_imgcodecs.so
ql: /usr/local/lib/aarch64-linux-gnu/libopencv_imgproc.so.4.6.0
ql: /usr/local/lib/aarch64-linux-gnu/libopencv_highgui.so.4.6.0
ql: /lib/aarch64-linux-gnu/libpthread.so.0
ql: CMakeFiles/ql.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/padmin/Documents/Projects/quick_labeling/ql/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ql"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ql.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ql.dir/build: ql

.PHONY : CMakeFiles/ql.dir/build

CMakeFiles/ql.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ql.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ql.dir/clean

CMakeFiles/ql.dir/depend:
	cd /home/padmin/Documents/Projects/quick_labeling/ql/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/padmin/Documents/Projects/quick_labeling/ql /home/padmin/Documents/Projects/quick_labeling/ql /home/padmin/Documents/Projects/quick_labeling/ql/build /home/padmin/Documents/Projects/quick_labeling/ql/build /home/padmin/Documents/Projects/quick_labeling/ql/build/CMakeFiles/ql.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ql.dir/depend

