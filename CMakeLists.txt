################
### reVision ###
################
cmake_minimum_required(VERSION 3.10)
project(reVision VERSION 4.1.9 DESCRIPTION "Video game engine project.")

# Get dependency directory locations from the user
set(ASSIMP_DIR "" CACHE PATH "ASSIMP root directory")
set(BULLET_DIR "" CACHE PATH "Bullet Physics library (BT) root directory")
set(FREEIMAGE_DIR "" CACHE PATH "FreeImage root directory")
set(GLFW_DIR "" CACHE PATH "OpenGL Framework library (GLFW) root directory")
set(GLM_DIR "" CACHE PATH "OpenGL Mathematics library (GLM) root directory")
set(SOLOUD_DIR "" CACHE PATH "SoLoud Audio Engine root directory")
set(CORE_DIR "${PROJECT_SOURCE_DIR}/src")
set(PROJECT_BIN ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

# Include directories of dependencies for entire project
include_directories( 	
	${CORE_DIR} 
)
include_directories( SYSTEM
	${ASSIMP_DIR}/include/
	${BULLET_DIR}/src/			
	${FREEIMAGE_DIR}/Source/
	${GLFW_DIR}/include/
	${GLM_DIR} 
	${SOLOUD_DIR}/include/
)

# Add external dependencies						
link_libraries(	
	debug		${ASSIMP_DIR}/lib/Debug/assimp.lib
	optimized	${ASSIMP_DIR}/lib/Release/assimp.lib
	debug		${ASSIMP_DIR}/lib/Debug/IrrXML.lib
	optimized	${ASSIMP_DIR}/lib/Release/IrrXML.lib
	debug		${ASSIMP_DIR}/lib/Debug/zlibstaticd.lib
	optimized	${ASSIMP_DIR}/lib/Release/zlibstatic.lib									
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
	debug		${FREEIMAGE_DIR}/Dist/x64/FreeImaged.lib
	optimized	${FREEIMAGE_DIR}/Dist/x64/FreeImage.lib
	debug		${GLFW_DIR}/src/Debug/glfw3.lib
	optimized	${GLFW_DIR}/src/Release/glfw3.lib
	debug		${GLM_DIR}/glm/Debug/glm_static.lib
	optimized	${GLM_DIR}/glm/Release/glm_static.lib									
	debug		${SOLOUD_DIR}/lib/soloud_static_x64_d.lib
	optimized	${SOLOUD_DIR}/lib/soloud_static_x64.lib
	"OpenGL32.lib"
)

# add all sub-projects and plugins here
add_subdirectory( src )

# Show option to generate docs if doxygen is found
find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
	option(GENERATE_DOCS "Generate documentation using Doxygen" true)
	if (GENERATE_DOCS)		
		set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/docs/)
		set(DOXYGEN_MARKDOWN_SUPPORT YES)
		set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
		set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
		set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
		doxygen_add_docs(Documentation ${PROJECT_SOURCE_DIR} COMMENT "Generate docs")
	endif (GENERATE_DOCS)
endif (DOXYGEN_FOUND)
 
# Visual studio specific setting: make the engine the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reVision)
# Set Global Properties
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set_property (GLOBAL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)app")
set_property (GLOBAL PROPERTY USE_FOLDERS ON)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Other)