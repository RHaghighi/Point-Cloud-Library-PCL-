# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/reza/PCL_Workspace/PMF_segmentation

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/reza/PCL_Workspace/PMF_segmentation/build

# Include any dependencies generated for this target.
include CMakeFiles/pmf.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/pmf.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pmf.dir/flags.make

CMakeFiles/pmf.dir/pmf.cpp.o: CMakeFiles/pmf.dir/flags.make
CMakeFiles/pmf.dir/pmf.cpp.o: ../pmf.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/reza/PCL_Workspace/PMF_segmentation/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/pmf.dir/pmf.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/pmf.dir/pmf.cpp.o -c /home/reza/PCL_Workspace/PMF_segmentation/pmf.cpp

CMakeFiles/pmf.dir/pmf.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/pmf.dir/pmf.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/reza/PCL_Workspace/PMF_segmentation/pmf.cpp > CMakeFiles/pmf.dir/pmf.cpp.i

CMakeFiles/pmf.dir/pmf.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/pmf.dir/pmf.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/reza/PCL_Workspace/PMF_segmentation/pmf.cpp -o CMakeFiles/pmf.dir/pmf.cpp.s

CMakeFiles/pmf.dir/pmf.cpp.o.requires:
.PHONY : CMakeFiles/pmf.dir/pmf.cpp.o.requires

CMakeFiles/pmf.dir/pmf.cpp.o.provides: CMakeFiles/pmf.dir/pmf.cpp.o.requires
	$(MAKE) -f CMakeFiles/pmf.dir/build.make CMakeFiles/pmf.dir/pmf.cpp.o.provides.build
.PHONY : CMakeFiles/pmf.dir/pmf.cpp.o.provides

CMakeFiles/pmf.dir/pmf.cpp.o.provides.build: CMakeFiles/pmf.dir/pmf.cpp.o

# Object files for target pmf
pmf_OBJECTS = \
"CMakeFiles/pmf.dir/pmf.cpp.o"

# External object files for target pmf
pmf_EXTERNAL_OBJECTS =

pmf: CMakeFiles/pmf.dir/pmf.cpp.o
pmf: CMakeFiles/pmf.dir/build.make
pmf: /usr/lib/x86_64-linux-gnu/libboost_system.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_thread.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_iostreams.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_mpi.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
pmf: /usr/lib/x86_64-linux-gnu/libpthread.so
pmf: /usr/local/lib/libpcl_common.so
pmf: /usr/lib/x86_64-linux-gnu/libflann_cpp_s.a
pmf: /usr/local/lib/libpcl_kdtree.so
pmf: /usr/local/lib/libpcl_octree.so
pmf: /usr/local/lib/libpcl_search.so
pmf: /usr/lib/libOpenNI.so
pmf: /usr/lib/libvtkCommon.so.5.8.0
pmf: /usr/lib/libvtkRendering.so.5.8.0
pmf: /usr/lib/libvtkHybrid.so.5.8.0
pmf: /usr/lib/libvtkCharts.so.5.8.0
pmf: /usr/local/lib/libpcl_io.so
pmf: /usr/local/lib/libpcl_visualization.so
pmf: /usr/local/lib/libpcl_sample_consensus.so
pmf: /usr/local/lib/libpcl_filters.so
pmf: /usr/local/lib/libpcl_features.so
pmf: /usr/local/lib/libpcl_keypoints.so
pmf: /usr/lib/libqhull.so
pmf: /usr/local/lib/libpcl_surface.so
pmf: /usr/local/lib/libpcl_registration.so
pmf: /usr/local/lib/libpcl_segmentation.so
pmf: /usr/local/lib/libpcl_recognition.so
pmf: /usr/local/lib/libpcl_people.so
pmf: /usr/local/lib/libpcl_outofcore.so
pmf: /usr/local/lib/libpcl_tracking.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_system.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_thread.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_iostreams.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_mpi.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
pmf: /usr/lib/x86_64-linux-gnu/libpthread.so
pmf: /usr/lib/libqhull.so
pmf: /usr/lib/libOpenNI.so
pmf: /usr/lib/x86_64-linux-gnu/libflann_cpp_s.a
pmf: /usr/lib/libvtkCommon.so.5.8.0
pmf: /usr/lib/libvtkRendering.so.5.8.0
pmf: /usr/lib/libvtkHybrid.so.5.8.0
pmf: /usr/lib/libvtkCharts.so.5.8.0
pmf: /usr/lib/x86_64-linux-gnu/libboost_system.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_thread.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_iostreams.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_mpi.so
pmf: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
pmf: /usr/lib/x86_64-linux-gnu/libpthread.so
pmf: /usr/local/lib/libpcl_common.so
pmf: /usr/local/lib/libpcl_kdtree.so
pmf: /usr/local/lib/libpcl_octree.so
pmf: /usr/local/lib/libpcl_search.so
pmf: /usr/local/lib/libpcl_io.so
pmf: /usr/local/lib/libpcl_visualization.so
pmf: /usr/local/lib/libpcl_sample_consensus.so
pmf: /usr/local/lib/libpcl_filters.so
pmf: /usr/local/lib/libpcl_features.so
pmf: /usr/local/lib/libpcl_keypoints.so
pmf: /usr/local/lib/libpcl_surface.so
pmf: /usr/local/lib/libpcl_registration.so
pmf: /usr/local/lib/libpcl_segmentation.so
pmf: /usr/local/lib/libpcl_recognition.so
pmf: /usr/local/lib/libpcl_people.so
pmf: /usr/local/lib/libpcl_outofcore.so
pmf: /usr/local/lib/libpcl_tracking.so
pmf: /usr/lib/libvtkViews.so.5.8.0
pmf: /usr/lib/libvtkInfovis.so.5.8.0
pmf: /usr/lib/libvtkWidgets.so.5.8.0
pmf: /usr/lib/libvtkHybrid.so.5.8.0
pmf: /usr/lib/libvtkParallel.so.5.8.0
pmf: /usr/lib/libvtkVolumeRendering.so.5.8.0
pmf: /usr/lib/libvtkRendering.so.5.8.0
pmf: /usr/lib/libvtkGraphics.so.5.8.0
pmf: /usr/lib/libvtkImaging.so.5.8.0
pmf: /usr/lib/libvtkIO.so.5.8.0
pmf: /usr/lib/libvtkFiltering.so.5.8.0
pmf: /usr/lib/libvtkCommon.so.5.8.0
pmf: /usr/lib/libvtksys.so.5.8.0
pmf: CMakeFiles/pmf.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable pmf"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pmf.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pmf.dir/build: pmf
.PHONY : CMakeFiles/pmf.dir/build

CMakeFiles/pmf.dir/requires: CMakeFiles/pmf.dir/pmf.cpp.o.requires
.PHONY : CMakeFiles/pmf.dir/requires

CMakeFiles/pmf.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pmf.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pmf.dir/clean

CMakeFiles/pmf.dir/depend:
	cd /home/reza/PCL_Workspace/PMF_segmentation/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/reza/PCL_Workspace/PMF_segmentation /home/reza/PCL_Workspace/PMF_segmentation /home/reza/PCL_Workspace/PMF_segmentation/build /home/reza/PCL_Workspace/PMF_segmentation/build /home/reza/PCL_Workspace/PMF_segmentation/build/CMakeFiles/pmf.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pmf.dir/depend
