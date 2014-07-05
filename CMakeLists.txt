# Require a recent version of cmake
cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)
# CMake entry point
cmake_minimum_required (VERSION 2.6)
project (PhysicsEngine)

find_package(OpenGL REQUIRED)
find_package(BOOST REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-1c73e35")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/external)

#include(FindSDL2 REQUIRED)

include_directories(
	external/AntTweakBar-1.16/include/
	external/glfw-2.7.6/include/
	external/glm-0.9.4.0/
	external/glew-1.9.0/include/
	external/assimp-3.0.1270/include/
	external/bullet-2.81-rev2613/src/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_276
	GLEW_190
	assimp
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)


# This adds the HPX cmake configuration directory to the search path.
#set(CMAKE_MODULE_PATH $ENV{HPX_DIR}/share/cmake-2.8/Modules)

# Instruct cmake to find the HPX settings
#find_package(HPX)

#include_directories(${HPX_INCLUDE_DIR})
#link_directories(${HPX_LIBRARY_DIR})


set (sources 
	physics/main.cpp
	physics/headers.hpp
	physics/geometry.hpp
    	physics/geometry.cpp
	physics/graphics.hpp
    	physics/graphics.cpp
	physics/pathing.hpp
	physics/pathing.cpp
	physics/runtime.hpp
     	physics/runtime.cpp
	physics/vert.glsl
	physics/frag.glsl
	common/controls.cpp
	common/controls.hpp
	common/nanoflann.hpp
	common/objloader.cpp
	common/objloader.hpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp)



add_executable(physics
		${sources})

target_link_libraries(physics
	${ALL_LIBS}
)

create_target_launcher(physics WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/physics/")
SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )

# add example executable
#add_hpx_executable(physics SOURCES ${sources} ${physics_FLAGS} FOLDER COMPONENT_DEPENDENCIES ${ALL_LIBS} assimp)
#create_target_launcher(physics_exe WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/physics/")

