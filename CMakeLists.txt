cmake_minimum_required(VERSION 3.0)
# Project Name: Delta
project(Delta)

# Get dependency directory locations from the user
set (GLM_DIR "" CACHE PATH "OpenGL Mathematics library (GLM) root directory")
set (GLEW_DIR "" CACHE PATH "OpenGL Extension Wrangler library (GLEW) root directory")
set (GLFW_DIR "" CACHE PATH "OpenGL Framework library (GLFW) root directory")
set (BULLET_DIR "" CACHE PATH "Bullet Physics library (BT) root directory")
set (CORE_DIR "${PROJECT_SOURCE_DIR}/src/Core")
set (PROJECT_BIN ${CMAKE_SOURCE_DIR})

# Include directories of dependencies for entire project
include_directories ( 	${CORE_DIR} 
						${GLM_DIR} 
						${GLEW_DIR}/include/ 
						${GLFW_DIR}/include/
						${BULLET_DIR}/src/)
						
link_libraries 	(					debug 		${GLM_DIR}/glm/Debug/glm_static.lib
									optimized	${GLM_DIR}/glm/Release/glm_static.lib 
												${GLEW_DIR}/lib/Release/x64/glew32s.lib
									debug		${GLFW_DIR}/src/Debug/glfw3.lib
									optimized	${GLFW_DIR}/src/Release/glfw3.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet2FileLoader.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Collision.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Common.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Dynamics.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3Geometry.lib
									debug		${BULLET_DIR}/lib/Debug/Bullet3OpenCL_clew.lib
									debug		${BULLET_DIR}/lib/Debug/BulletCollision.lib
									debug		${BULLET_DIR}/lib/Debug/BulletDynamics.lib
									debug		${BULLET_DIR}/lib/Debug/BulletFileLoader.lib
									debug		${BULLET_DIR}/lib/Debug/BulletInverseDynamics.lib
									debug		${BULLET_DIR}/lib/Debug/BulletInverseDynamicsUtils.lib
									debug		${BULLET_DIR}/lib/Debug/BulletRobotics.lib
									debug		${BULLET_DIR}/lib/Debug/BulletSoftBody.lib
									debug		${BULLET_DIR}/lib/Debug/BulletWorldImporter.lib
									debug		${BULLET_DIR}/lib/Debug/BulletXmlWorldImporter.lib
									debug		${BULLET_DIR}/lib/Debug/ConvexDecomposition.lib
									debug		${BULLET_DIR}/lib/Debug/GIMPACTUtils.lib
									debug		${BULLET_DIR}/lib/Debug/HACD.lib
									debug		${BULLET_DIR}/lib/Debug/LinearMath.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet2FileLoader.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Collision.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Common.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Dynamics.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3Geometry.lib
									optimized	${BULLET_DIR}/lib/Release/Bullet3OpenCL_clew.lib
									optimized	${BULLET_DIR}/lib/Release/BulletCollision.lib
									optimized	${BULLET_DIR}/lib/Release/BulletDynamics.lib
									optimized	${BULLET_DIR}/lib/Release/BulletFileLoader.lib
									optimized	${BULLET_DIR}/lib/Release/BulletInverseDynamics.lib
									optimized	${BULLET_DIR}/lib/Release/BulletInverseDynamicsUtils.lib
									optimized	${BULLET_DIR}/lib/Release/BulletRobotics.lib
									optimized	${BULLET_DIR}/lib/Release/BulletSoftBody.lib
									optimized	${BULLET_DIR}/lib/Release/BulletWorldImporter.lib
									optimized	${BULLET_DIR}/lib/Release/BulletXmlWorldImporter.lib
									optimized	${BULLET_DIR}/lib/Release/ConvexDecomposition.lib
									optimized	${BULLET_DIR}/lib/Release/GIMPACTUtils.lib
									optimized	${BULLET_DIR}/lib/Release/HACD.lib
									optimized	${BULLET_DIR}/lib/Release/LinearMath.lib
												"OpenGL32.lib")

# add all sub-projects and plugins here
add_subdirectory( src/Core )
add_subdirectory( src/Plugins/FreeImage )
add_subdirectory( src/Plugins/ASSIMP )

# Generates an "includes" folder with only the headers
macro(GENERATE_INCLUDES GLOBPAT)
	file(GLOB_RECURSE COPY_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${GLOBPAT})
	add_custom_target(Generate_Includes ALL COMMENT "Copying files")
	foreach(FILENAME ${COPY_FILES}) 	
		set(SRC "src/${FILENAME}")
		set(DST "include/DT/${FILENAME}")	
		add_custom_command(
			TARGET Generate_Includes
			COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
		)
	endforeach()	
endmacro()
GENERATE_INCLUDES("src/*.h")	
 
# Enable folder structure
set_property (GLOBAL PROPERTY USE_FOLDERS ON)
# ALL_BUILD, ZERO_CHECK, and other build functions' folder
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Build_Functions)
set_property (TARGET Generate_Includes PROPERTY FOLDER "Build_Functions")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")