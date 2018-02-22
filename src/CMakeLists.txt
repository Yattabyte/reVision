# Get source files for this project
file (GLOB_RECURSE ROOT RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.h")
# Generate source groups mimicking the folder structure
foreach(source IN LISTS ROOT)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()


############
#   EXEC   #
############
set (Module dt_exec)
# Create a library using those source files	
add_executable			(${Module} 	${ROOT})	
# Set working directory to the project directory
set_target_properties 	(${Module} PROPERTIES 	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
target_compile_Definitions (${Module} PRIVATE ENGINE_EXE_EXPORT)
set_target_properties(${Module} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)$(Configuration)")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT dt_exec)

############
#   DLL    #
############
set (Module dt_lib)
list (REMOVE_ITEM ROOT "main.cpp")
# Create a library using those source files
add_library				(${Module} SHARED 	${ROOT})	
# Set working directory to the project directory
set_target_properties 	(${Module} PROPERTIES 	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
												PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
target_compile_definitions (${Module} PRIVATE ENGINE_DLL_EXPORT)
