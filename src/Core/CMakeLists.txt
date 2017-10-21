############
#   CORE   #
############
set (Module dt_Core)
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
# Link .lib files of dependencies
target_link_libraries 	(${Module} 	${GLEW_LIB_DIR}
									${GLFW_LIB_DIR}
									"OpenGL32.lib")
# Set working directory to the project directory
set_target_properties 	(${Module} PROPERTIES 	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
target_compile_Definitions (${Module} PRIVATE DT_CORE_EXPORT)