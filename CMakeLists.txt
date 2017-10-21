cmake_minimum_required(VERSION 3.0)
# Project Name: Delta
project(Delta)

# Get dependency directory locations from the user
set (GLM_DIR "" CACHE PATH "OpenGL Mathematics library (GLM) include directory")
set (GLEW_DIR "" CACHE PATH "OpenGL Extension Wrangler library (GLEW) include directory")
set (GLFW_DIR "" CACHE PATH "OpenGL Framework library (GLFW) include directory")
set (GLEW_LIB_DIR "" CACHE FILE_PATH "GLEW .lib path")
set (GLFW_LIB_DIR "" CACHE FILE_PATH "GLFW .lib path")
set (CORE_DIR "${PROJECT_SOURCE_DIR}/src/Core")
set (PROJECT_BIN ${CMAKE_SOURCE_DIR})

# Include directories of dependencies for entire project
include_directories ( ${CORE_DIR} ${GLM_DIR} ${GLEW_DIR} ${GLFW_DIR} )

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
