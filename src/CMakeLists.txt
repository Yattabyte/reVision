# Get source files for this project
file (GLOB_RECURSE ROOT RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.c" "*.h" "*.rc")
# Generate source groups mimicking the folder structure
foreach(source IN LISTS ROOT)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
configure_file(Engine.h.in Engine.h)


############
#   EXEC   #
############
set(Module reVision)
# Create a library using those source files	
add_executable(${Module} 	${ROOT})	
# Set working directory to the project directory
set_target_properties(${Module} PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
	PDB_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
)
target_compile_Definitions(${Module}	
	PRIVATE		$<$<CONFIG:DEBUG>:DEBUG>
	PUBLIC		FREEIMAGE_LIB
)
set_target_properties(${Module} PROPERTIES 
	VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)app"
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reVision)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()