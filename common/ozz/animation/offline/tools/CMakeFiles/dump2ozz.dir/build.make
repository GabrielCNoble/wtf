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

# Include any dependencies generated for this target.
include src/animation/offline/tools/CMakeFiles/dump2ozz.dir/depend.make

# Include the progress variables for this target.
include src/animation/offline/tools/CMakeFiles/dump2ozz.dir/progress.make

# Include the compile flags for this target's objects.
include src/animation/offline/tools/CMakeFiles/dump2ozz.dir/flags.make

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/flags.make
src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/includes_CXX.rsp
src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/src/animation/offline/tools/dump2ozz.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="N:\Files\Gabriel Noble\Compiler\ozz-compiled\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && C:\PROGRA~2\Dev-Cpp\MinGW64\bin\G__~1.EXE   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles\dump2ozz.dir\dump2ozz.cc.obj -c "N:\Files\Gabriel Noble\Compiler\ozz-animation\src\animation\offline\tools\dump2ozz.cc"

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dump2ozz.dir/dump2ozz.cc.i"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && C:\PROGRA~2\Dev-Cpp\MinGW64\bin\G__~1.EXE  $(CXX_DEFINES) $(CXX_FLAGS) -E "N:\Files\Gabriel Noble\Compiler\ozz-animation\src\animation\offline\tools\dump2ozz.cc" > CMakeFiles\dump2ozz.dir\dump2ozz.cc.i

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dump2ozz.dir/dump2ozz.cc.s"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && C:\PROGRA~2\Dev-Cpp\MinGW64\bin\G__~1.EXE  $(CXX_DEFINES) $(CXX_FLAGS) -S "N:\Files\Gabriel Noble\Compiler\ozz-animation\src\animation\offline\tools\dump2ozz.cc" -o CMakeFiles\dump2ozz.dir\dump2ozz.cc.s

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.requires:

.PHONY : src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.requires

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.provides: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.requires
	$(MAKE) -f src\animation\offline\tools\CMakeFiles\dump2ozz.dir\build.make src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.provides.build
.PHONY : src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.provides

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.provides.build: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj


# Object files for target dump2ozz
dump2ozz_OBJECTS = \
"CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj"

# External object files for target dump2ozz
dump2ozz_EXTERNAL_OBJECTS =

src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/build.make
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/libozz_animation_tools.a
src/animation/offline/tools/dump2ozz.exe: src/options/libozz_options.a
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/libozz_animation_offline.a
src/animation/offline/tools/dump2ozz.exe: src/animation/runtime/libozz_animation.a
src/animation/offline/tools/dump2ozz.exe: src/base/libozz_base.a
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/json/libjson.a
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/linklibs.rsp
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/objects1.rsp
src/animation/offline/tools/dump2ozz.exe: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="N:\Files\Gabriel Noble\Compiler\ozz-compiled\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable dump2ozz.exe"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\dump2ozz.dir\link.txt --verbose=$(VERBOSE)
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && .\dump2ozz.exe "--file=N:/Files/Gabriel Noble/Compiler/ozz-animation/src/animation/offline/tools/dump2ozz.cc" "--config={\"skeleton\": {\"import\":{\"enable\":false}},\"animations\":[]}" "--config_dump_reference=N:/Files/Gabriel Noble/Compiler/ozz-animation/src/animation/offline/tools/reference.json"

# Rule to build all files generated by this target.
src/animation/offline/tools/CMakeFiles/dump2ozz.dir/build: src/animation/offline/tools/dump2ozz.exe

.PHONY : src/animation/offline/tools/CMakeFiles/dump2ozz.dir/build

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/requires: src/animation/offline/tools/CMakeFiles/dump2ozz.dir/dump2ozz.cc.obj.requires

.PHONY : src/animation/offline/tools/CMakeFiles/dump2ozz.dir/requires

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/clean:
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools && $(CMAKE_COMMAND) -P CMakeFiles\dump2ozz.dir\cmake_clean.cmake
.PHONY : src/animation/offline/tools/CMakeFiles/dump2ozz.dir/clean

src/animation/offline/tools/CMakeFiles/dump2ozz.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "N:\Files\Gabriel Noble\Compiler\ozz-animation" "N:\Files\Gabriel Noble\Compiler\ozz-animation\src\animation\offline\tools" "N:\Files\Gabriel Noble\Compiler\ozz-compiled" "N:\Files\Gabriel Noble\Compiler\ozz-compiled\src\animation\offline\tools" "N:\Files\Gabriel Noble\Compiler\ozz-compiled\src\animation\offline\tools\CMakeFiles\dump2ozz.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : src/animation/offline/tools/CMakeFiles/dump2ozz.dir/depend
