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
include src/animation/offline/tools/json/CMakeFiles/json.dir/depend.make

# Include the progress variables for this target.
include src/animation/offline/tools/json/CMakeFiles/json.dir/progress.make

# Include the compile flags for this target's objects.
include src/animation/offline/tools/json/CMakeFiles/json.dir/flags.make

src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj: src/animation/offline/tools/json/CMakeFiles/json.dir/flags.make
src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj: src/animation/offline/tools/json/CMakeFiles/json.dir/includes_CXX.rsp
src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj: N:/Files/Gabriel\ Noble/Compiler/ozz-animation/extern/jsoncpp/dist/jsoncpp.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="N:\Files\Gabriel Noble\Compiler\ozz-compiled\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools\json && C:\PROGRA~2\Dev-Cpp\MinGW64\bin\G__~1.EXE   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles\json.dir\dist\jsoncpp.cpp.obj -c "N:\Files\Gabriel Noble\Compiler\ozz-animation\extern\jsoncpp\dist\jsoncpp.cpp"

src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/json.dir/dist/jsoncpp.cpp.i"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools\json && C:\PROGRA~2\Dev-Cpp\MinGW64\bin\G__~1.EXE  $(CXX_DEFINES) $(CXX_FLAGS) -E "N:\Files\Gabriel Noble\Compiler\ozz-animation\extern\jsoncpp\dist\jsoncpp.cpp" > CMakeFiles\json.dir\dist\jsoncpp.cpp.i

src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/json.dir/dist/jsoncpp.cpp.s"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools\json && C:\PROGRA~2\Dev-Cpp\MinGW64\bin\G__~1.EXE  $(CXX_DEFINES) $(CXX_FLAGS) -S "N:\Files\Gabriel Noble\Compiler\ozz-animation\extern\jsoncpp\dist\jsoncpp.cpp" -o CMakeFiles\json.dir\dist\jsoncpp.cpp.s

src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.requires:

.PHONY : src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.requires

src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.provides: src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.requires
	$(MAKE) -f src\animation\offline\tools\json\CMakeFiles\json.dir\build.make src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.provides.build
.PHONY : src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.provides

src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.provides.build: src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj


# Object files for target json
json_OBJECTS = \
"CMakeFiles/json.dir/dist/jsoncpp.cpp.obj"

# External object files for target json
json_EXTERNAL_OBJECTS =

src/animation/offline/tools/json/libjson.a: src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj
src/animation/offline/tools/json/libjson.a: src/animation/offline/tools/json/CMakeFiles/json.dir/build.make
src/animation/offline/tools/json/libjson.a: src/animation/offline/tools/json/CMakeFiles/json.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="N:\Files\Gabriel Noble\Compiler\ozz-compiled\CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libjson.a"
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools\json && $(CMAKE_COMMAND) -P CMakeFiles\json.dir\cmake_clean_target.cmake
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools\json && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\json.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/animation/offline/tools/json/CMakeFiles/json.dir/build: src/animation/offline/tools/json/libjson.a

.PHONY : src/animation/offline/tools/json/CMakeFiles/json.dir/build

src/animation/offline/tools/json/CMakeFiles/json.dir/requires: src/animation/offline/tools/json/CMakeFiles/json.dir/dist/jsoncpp.cpp.obj.requires

.PHONY : src/animation/offline/tools/json/CMakeFiles/json.dir/requires

src/animation/offline/tools/json/CMakeFiles/json.dir/clean:
	cd /d N:\Files\GABRIE~1\Compiler\OZZ-CO~1\src\ANIMAT~1\offline\tools\json && $(CMAKE_COMMAND) -P CMakeFiles\json.dir\cmake_clean.cmake
.PHONY : src/animation/offline/tools/json/CMakeFiles/json.dir/clean

src/animation/offline/tools/json/CMakeFiles/json.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "N:\Files\Gabriel Noble\Compiler\ozz-animation" "N:\Files\Gabriel Noble\Compiler\ozz-animation\extern\jsoncpp" "N:\Files\Gabriel Noble\Compiler\ozz-compiled" "N:\Files\Gabriel Noble\Compiler\ozz-compiled\src\animation\offline\tools\json" "N:\Files\Gabriel Noble\Compiler\ozz-compiled\src\animation\offline\tools\json\CMakeFiles\json.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : src/animation/offline/tools/json/CMakeFiles/json.dir/depend
