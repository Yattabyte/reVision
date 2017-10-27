#########################
#   Plugin: ASSIMP      #
#########################
set (Module dt_ASSIMP)
set (ASSIMP_DIR "" CACHE PATH "ASSIMP root directory")
# Get source files for this project
file (GLOB 			ROOT 		"*.cpp" "*.h" )
file (GLOB_RECURSE 	ASSETS 		"Assets/*.cpp" "Assets/*.h")
file (GLOB_RECURSE 	MANAGERS 	"Managers/*.cpp" "Managers/*.h")
file (GLOB_RECURSE 	RENDERING 	"Rendering/*.cpp" "Rendering/*.h")
file (GLOB_RECURSE 	UTILITIES 	"Utilities/*.cpp" "Utilities/*.h")
# Organize the source files into folders/groups
source_group ("" 			FILES	${ROOT})
source_group ("Assets" 		FILES 	${ASSETS})
source_group ("Managers" 	FILES 	${MANAGERS} )
source_group ("Rendering" 	FILES 	${RENDERING} )
source_group ("Utilities" 	FILES 	${UTILITIES} ) 
# Create a library using those source files
add_library				(${Module} SHARED 	${ROOT}
											${ASSETS}
											${MANAGERS}
											${RENDERING}
											${UTILITIES})										
# Include directories of dependencies
include_directories 	("." ${ASSIMP_DIR}/include/ ../${MODULE_IMAGE}/.)
# Link .lib files of dependencies
target_link_libraries 	(${Module} 				dt_Core
												${MODULE_IMAGE}
									debug		${ASSIMP_DIR}/lib/Debug/assimp-vc140-mt.lib
									optimized	${ASSIMP_DIR}/lib/Release/assimp-vc140-mt.lib)
# Set working directory to the project directory
set_target_properties 	(${Module} PROPERTIES 	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												FOLDER "Plugins")
target_compile_Definitions (${Module} PRIVATE DT_ASSIMP_EXPORT)
if (NOT MODULE_IMAGE)
	message( FATAL_ERROR "dt_ASSIMP requires some form of image loading to be present, but no image loader is being built!" )
endif()