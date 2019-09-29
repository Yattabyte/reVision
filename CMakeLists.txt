################
### reVision ###
################
cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
project(reVision VERSION 4.13.9 DESCRIPTION "Video game engine project.")

# Get dependency directory locations from the user
set(CUSTOM_FREEIMAGE "" CACHE PATH "FreeImage root directory")
set(CUSTOM_SOLOUD "" CACHE PATH "SoLoud Audio Engine root directory")
set(CORE_DIR "${PROJECT_SOURCE_DIR}/src")
set(PROJECT_BIN ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Oi /Ot /GF /Qpar /std:c++latest /GL")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Gy /JMC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /GL")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG:incremental")
set_property(GLOBAL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)app")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Other)
include_directories(SYSTEM ${CORE_DIR} ${CUSTOM_FREEIMAGE}/Source/ ${CUSTOM_SOLOUD}/include/)
link_libraries(		
	debug		${CUSTOM_FREEIMAGE}/Dist/x64/FreeImaged.lib
	optimized	${CUSTOM_FREEIMAGE}/Dist/x64/FreeImage.lib					
	debug		${CUSTOM_SOLOUD}/lib/soloud_static_x64_d.lib
	optimized	${CUSTOM_SOLOUD}/lib/soloud_static_x64.lib
	"OpenGL32.lib"
)


######################
### INCLUDE ASSIMP ###
######################
set(CUSTOM_ASSIMP "" CACHE PATH "ASSIMP root directory")
if (CUSTOM_ASSIMP STREQUAL "")
	ExternalProject_Add(
		ASSIMP
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/			
		STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/stamps/
		BUILD_IN_SOURCE ON
		GIT_REPOSITORY https://github.com/assimp/assimp
		GIT_TAG master
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>	
			-DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT:BOOL=ON
			-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT:BOOL=ON
			-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
			-DASSIMP_BUILD_SAMPLES:BOOL=OFF
			-DASSIMP_BUILD_TESTS:BOOL=OFF
			-DASSIMP_NO_EXPORT:BOOL=ON
			-DBUILD_SHARED_LIBS:BOOL=OFF
			-DBUILD_STATIC_LIBS:BOOL=ON
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
			-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
			-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
			-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(ASSIMP SOURCE_DIR)
	set (CUSTOM_ASSIMP ${SOURCE_DIR})	
endif (CUSTOM_ASSIMP STREQUAL "")	
include_directories(SYSTEM ${CUSTOM_ASSIMP}/include/)
link_libraries(	
	debug		${CUSTOM_ASSIMP}/lib/Debug/assimp-vc142-mtd.lib
	optimized	${CUSTOM_ASSIMP}/lib/Release/assimp-vc142-mt.lib
	debug		${CUSTOM_ASSIMP}/lib/Debug/IrrXMLd.lib
	optimized	${CUSTOM_ASSIMP}/lib/Release/IrrXML.lib
	debug		${CUSTOM_ASSIMP}/lib/Debug/zlibstaticd.lib
	optimized	${CUSTOM_ASSIMP}/lib/Release/zlibstatic.lib
)


######################
### INCLUDE BULLET ###
######################
set(CUSTOM_BULLET "" CACHE PATH "Bullet Physics library (BT) root directory")
if (CUSTOM_BULLET STREQUAL "")
	ExternalProject_Add(
		BULLET
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/			
		STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/stamps/
		BUILD_IN_SOURCE ON
		GIT_REPOSITORY https://github.com/bulletphysics/bullet3
		GIT_TAG master
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>	
			-DBUILD_BULLET2_DEMOS:BOOL=OFF
			-DBUILD_BULLET3:BOOL=ON
			-DBUILD_CLSOCKET:BOOL=OFF
			-DBUILD_CPU_DEMOS:BOOL=OFF
			-DBUILD_ENET:BOOL=OFF
			-DBUILD_EXTRAS:BOOL=OFF
			-DBUILD_OPENGL3_DEMOS:BOOL=OFF
			-DBUILD_PYBULLET:BOOL=OFF
			-DBUILD_SHARED_LIBS:BOOL=OFF
			-DBUILD_STATIC_LIBS:BOOL=ON
			-DBUILD_UNIT_TESTS:BOOL=OFF
			-DINSTALL_LIBS:BOOL=OFF
			-DUSE_MSVC_FUNCTION_LEVEL_LINKING:BOOL=ON
			-DUSE_MSVC_INCREMENTAL_LINKING:BOOL=ON
			-DUSE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=ON
			-DUSE_MSVC_STRING_POOLING:BOOL=ON
			-DUSE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD:BOOL=ON			
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
			-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
			-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
			-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(BULLET SOURCE_DIR)
	set (CUSTOM_BULLET ${SOURCE_DIR})	
endif (CUSTOM_BULLET STREQUAL  "")	
include_directories(SYSTEM ${CUSTOM_BULLET}/src/)
link_libraries(	
	debug		${CUSTOM_BULLET}/lib/Debug/Bullet2FileLoader_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/Bullet2FileLoader.lib
	debug		${CUSTOM_BULLET}/lib/Debug/Bullet3Collision_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/Bullet3Collision.lib
	debug		${CUSTOM_BULLET}/lib/Debug/Bullet3Common_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/Bullet3Common.lib
	debug		${CUSTOM_BULLET}/lib/Debug/Bullet3Dynamics_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/Bullet3Dynamics.lib
	debug		${CUSTOM_BULLET}/lib/Debug/Bullet3Geometry_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/Bullet3Geometry.lib
	debug		${CUSTOM_BULLET}/lib/Debug/Bullet3OpenCL_clew_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/Bullet3OpenCL_clew.lib
	debug		${CUSTOM_BULLET}/lib/Debug/BulletCollision_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/BulletCollision.lib
	debug		${CUSTOM_BULLET}/lib/Debug/BulletDynamics_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/BulletDynamics.lib
	debug		${CUSTOM_BULLET}/lib/Debug/BulletInverseDynamics_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/BulletInverseDynamics.lib
	debug		${CUSTOM_BULLET}/lib/Debug/BulletSoftBody_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/BulletSoftBody.lib
	debug		${CUSTOM_BULLET}/lib/Debug/LinearMath_Debug.lib
	optimized	${CUSTOM_BULLET}/lib/Release/LinearMath.lib
)

####################
### INCLUDE GLAD ###
####################
ExternalProject_Add(
	GLAD
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/			
	SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/src/glad/
	STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/stamps/
	BUILD_IN_SOURCE ON
	CONFIGURE_COMMAND ""
	UPDATE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
ExternalProject_Get_Property(GLAD SOURCE_DIR)
include_directories(SYSTEM ${SOURCE_DIR})


####################
### INCLUDE GLFW ###
####################
set(CUSTOM_GLFW "" CACHE PATH "OpenGL Framework library (GLFW) root directory")
if (CUSTOM_GLFW STREQUAL "")
	ExternalProject_Add(
		GLFW
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/			
		STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/stamps/
		BUILD_IN_SOURCE ON
		GIT_REPOSITORY https://github.com/glfw/glfw
		GIT_TAG master
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>			
			-DBUILD_SHARED_LIBS:BOOL=OFF
			-DBUILD_STATIC_LIBS:BOOL=ON
			-DINSTALL_LIBS:BOOL=OFF
			-DGLFW_BUILD_DOCS:BOOL=OFF			
			-DGLFW_BUILD_EXAMPLES:BOOL=OFF
			-DGLFW_BUILD_TESTS:BOOL=OFF
			-DGLFW_INSTALL:BOOL=ON
			-DGLFW_USE_HYBRID_HPG:BOOL=ON
			-DUSE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=ON
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
			-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
			-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
			-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(GLFW SOURCE_DIR)
	set (CUSTOM_GLFW ${SOURCE_DIR})	
endif (CUSTOM_GLFW STREQUAL "")	
include_directories(SYSTEM ${CUSTOM_GLFW}/include/)
link_libraries(	
	debug		${CUSTOM_GLFW}/src/Debug/glfw3.lib
	optimized	${CUSTOM_GLFW}/src/Release/glfw3.lib		
)


###################
### INCLUDE GLM ###
###################
set(CUSTOM_GLM "" CACHE PATH "GLM root directory")
if (CUSTOM_GLM STREQUAL "")
	ExternalProject_Add(
		GLM
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/
		STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/stamps/
		BUILD_IN_SOURCE ON
		GIT_REPOSITORY https://github.com/g-truc/glm
		GIT_TAG master
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
			-DBUILD_SHARED_LIBS:BOOL=OFF
			-DBUILD_STATIC_LIBS:BOOL=ON
			-DGLM_TEST_ENABLE:BOOL=OFF
			-DGLM_TEST_ENABLE_CXX_17:BOOL=ON		
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
			-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
			-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
			-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(GLM SOURCE_DIR)
	set (CUSTOM_GLM ${SOURCE_DIR})	
endif (CUSTOM_GLM STREQUAL "")	
include_directories(SYSTEM ${CUSTOM_GLM}/)


#########################
### INCLUDE DearIMGUI ###
#########################
set(CUSTOM_IMGUI "" CACHE PATH "DearIMGUI root directory")
if (CUSTOM_IMGUI STREQUAL "")
	ExternalProject_Add(
		IMGUI
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/			
		STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/stamps/
		BUILD_IN_SOURCE ON
		GIT_REPOSITORY https://github.com/ocornut/imgui
		GIT_TAG docking
		CONFIGURE_COMMAND ""
		UPDATE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(IMGUI SOURCE_DIR)
	set (CUSTOM_IMGUI ${SOURCE_DIR})	
endif (CUSTOM_IMGUI STREQUAL "")	
include_directories(SYSTEM ${CUSTOM_IMGUI})


#####################
### DOXYGEN CHECK ###
#####################
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


# Add our own source files
add_subdirectory( src )

 
# Visual studio specific setting: make the engine the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reVision)