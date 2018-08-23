cmake_minimum_required(VERSION 3.0)
# Project Name: reVision
project(reVision)

# Get dependency directory locations from the user
set (ASSIMP_DIR "" CACHE PATH "ASSIMP root directory")
set (FREEIMAGE_DIR "" CACHE PATH "FreeImage root directory")
set (GLM_DIR "" CACHE PATH "OpenGL Mathematics library (GLM) root directory")
set (GLEW_DIR "" CACHE PATH "OpenGL Extension Wrangler library (GLEW) root directory")
set (GLFW_DIR "" CACHE PATH "OpenGL Framework library (GLFW) root directory")
set (BULLET_DIR "" CACHE PATH "Bullet Physics library (BT) root directory")
set (CORE_DIR "${PROJECT_SOURCE_DIR}/src")
set (PROJECT_BIN ${CMAKE_SOURCE_DIR})

# Include directories of dependencies for entire project
include_directories ( 	${CORE_DIR} 
						${ASSIMP_DIR}/include/
						${FREEIMAGE_DIR}/Source/
						${GLM_DIR} 
						${GLEW_DIR}/include/ 
						${GLFW_DIR}/include/
						${BULLET_DIR}/src/			)
						
link_libraries 	(					debug		${ASSIMP_DIR}/lib/Debug/assimp-vc140-mt.lib
									optimized	${ASSIMP_DIR}/lib/Release/assimp-vc140-mt.lib
									debug		${ASSIMP_DIR}/lib/Debug/IrrXML.lib
									optimized	${ASSIMP_DIR}/lib/Release/IrrXML.lib
									debug		${ASSIMP_DIR}/lib/Debug/zlibstaticd.lib
									optimized	${ASSIMP_DIR}/lib/Release/zlibstatic.lib
												${FREEIMAGE_DIR}/Dist/x64/FreeImage.lib
												${GLEW_DIR}/lib/Release/x64/glew32s.lib
									debug		${GLFW_DIR}/src/Debug/glfw3.lib
									optimized	${GLFW_DIR}/src/Release/glfw3.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet2FileLoader.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet2FileLoader.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Collision.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Collision.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Common.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Common.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Dynamics.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Dynamics.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Geometry.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Geometry.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3OpenCL_clew.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3OpenCL_clew.lib
									debug		${BULLET_DIR}/lib/Debug/BulletCollision.lib
									optimized	${BULLET_DIR}/lib/Release/BulletCollision.lib
									debug		${BULLET_DIR}/lib/Debug/BulletDynamics.lib
									optimized	${BULLET_DIR}/lib/Release/BulletDynamics.lib
									debug		${BULLET_DIR}/lib/Debug/BulletInverseDynamics.lib
									optimized	${BULLET_DIR}/lib/Release/BulletInverseDynamics.lib
									debug		${BULLET_DIR}/lib/Debug/BulletSoftBody.lib
									optimized	${BULLET_DIR}/lib/Release/BulletSoftBody.lib
									debug		${BULLET_DIR}/lib/Debug/LinearMath.lib
									optimized	${BULLET_DIR}/lib/Release/LinearMath.lib
												"OpenGL32.lib")
					
# add all sub-projects and plugins here
add_subdirectory( src )
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reVision)

# Generates the documentation folder and runs doxygen
macro(GENERATE_DOCUMENTATION)
	find_package(Doxygen)
	if(DOXYGEN_FOUND)
		# Create a new doxyfile with the appropriate directory
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out @ONLY)		
		# Create the folder for documentation to be stored in
		add_custom_target(Generate_Documentation COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Documentation/)				
		add_custom_command(TARGET Generate_Documentation COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile.out WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Documentation/ )		
	endif(DOXYGEN_FOUND)
endmacro()
GENERATE_DOCUMENTATION()	
 
# Enable folder structure
set_property (GLOBAL PROPERTY USE_FOLDERS ON)
# ALL_BUILD, ZERO_CHECK, and other build functions' folder
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Build_Functions)
set_property (TARGET Generate_Documentation PROPERTY FOLDER "Build_Functions")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")