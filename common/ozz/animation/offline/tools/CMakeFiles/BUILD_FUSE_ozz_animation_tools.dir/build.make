# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.3

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files (x86)\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files (x86)\CMake\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "N:\Files\Gabriel Noble\Compiler\ozz-animation"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "N:\Files\Gabriel Noble\Compiler\ozz-compiled"

# Utility rule file for BUILD_FUSE_ozz_animation_tools.

# Include the progress variables for this target.
include src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/progress.make

src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools: src_fused/ozz_animation_tools.cc


src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/include/ozz/animation/offline/tools/import2ozz.h
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz.cc
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz_anim.h
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz_anim.cc
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz_config.h
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz_config.cc
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz_skel.h
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/import2ozz_skel.cc
src_fused/ozz_animation_tools.cc: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/build-utils/cmake/fuse_target_script.cmake
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="N:\Files\Gabriel Noble\Compiler\ozz-compiled\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Generating ../../../../src_fused/ozz_animation_tools.cc"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && "C:\Program Files (x86)\CMake\bin\cmake.exe" -Dozz_fuse_output_file="N:/Files/Gabriel Noble/Compiler/ozz-compiled/src_fused/ozz_animation_tools.cc" -Dozz_target_source_files="N:/Files/Gabriel Noble/Compiler/ozz-animation/include/ozz/animation/offline/tools/import2ozz.h import2ozz.cc import2ozz_anim.h import2ozz_anim.cc import2ozz_config.h import2ozz_config.cc import2ozz_skel.h import2ozz_skel.cc" -Dozz_fuse_target_dir="N:/Files/Gabriel Noble/Compiler/ozz-animation/src/animation/offline/tools" -Dozz_fuse_src_dir="N:/Files/Gabriel Noble/Compiler/ozz-animation" -P "N:/Files/Gabriel Noble/Compiler/ozz-animation/build-utils/cmake/fuse_target_script.cmake"

BUILD_FUSE_ozz_animation_tools: src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools
BUILD_FUSE_ozz_animation_tools: src_fused/ozz_animation_tools.cc
BUILD_FUSE_ozz_animation_tools: src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/build.make

.PHONY : BUILD_FUSE_ozz_animation_tools

# Rule to build all files generated by this target.
src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/build: BUILD_FUSE_ozz_animation_tools

.PHONY : src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/build

src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/clean:
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && $(CMAKE_COMMAND) -P CMakeFiles\BUILD_FUSE_ozz_animation_tools.dir\cmake_clean.cmake
.PHONY : src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/clean

src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "N:\Files\Gabriel Noble\Compiler\ozz-animation" "N:\Files\Gabriel Noble\Compiler\ozz-animation\src\animation\offline\tools" "N:\Files\Gabriel Noble\Compiler\ozz-compiled" "N:\Files\Gabriel Noble\Compiler\ozz-compiled\src\animation\offline\tools" "N:\Files\Gabriel Noble\Compiler\ozz-compiled\src\animation\offline\tools\CMakeFiles\BUILD_FUSE_ozz_animation_tools.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : src/animation/offline/tools/CMakeFiles/BUILD_FUSE_ozz_animation_tools.dir/depend

