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
CMAKE_SOURCE_DIR = /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build

# Include any dependencies generated for this target.
include CMakeFiles/grabber.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/grabber.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/grabber.dir/flags.make

CMakeFiles/grabber.dir/grabber.cpp.o: CMakeFiles/grabber.dir/flags.make
CMakeFiles/grabber.dir/grabber.cpp.o: ../grabber.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/grabber.dir/grabber.cpp.o"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/grabber.dir/grabber.cpp.o -c /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/grabber.cpp

CMakeFiles/grabber.dir/grabber.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/grabber.dir/grabber.cpp.i"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/grabber.cpp > CMakeFiles/grabber.dir/grabber.cpp.i

CMakeFiles/grabber.dir/grabber.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/grabber.dir/grabber.cpp.s"
	/usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/grabber.cpp -o CMakeFiles/grabber.dir/grabber.cpp.s

# Object files for target grabber
grabber_OBJECTS = \
"CMakeFiles/grabber.dir/grabber.cpp.o"

# External object files for target grabber
grabber_EXTERNAL_OBJECTS =

grabber: CMakeFiles/grabber.dir/grabber.cpp.o
grabber: CMakeFiles/grabber.dir/build.make
grabber: /lib/aarch64-linux-gnu/libpthread.so.0
grabber: CMakeFiles/grabber.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable grabber"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/grabber.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/grabber.dir/build: grabber

.PHONY : CMakeFiles/grabber.dir/build

CMakeFiles/grabber.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/grabber.dir/cmake_clean.cmake
.PHONY : CMakeFiles/grabber.dir/clean

CMakeFiles/grabber.dir/depend:
	cd /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1 /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1 /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build /home/padmin/Documents/Projects/quick_labeling/OV9281_grabber_v1/build/CMakeFiles/grabber.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/grabber.dir/depend

