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
add_executable(${Module} ${ROOT})	
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
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT reVision)
set_target_properties(${Module} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)app")
target_sources(${Module} 
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/glad/glad/glad.c
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/IMGUI/imgui.cpp
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/IMGUI/imgui_demo.cpp
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/IMGUI/imgui_draw.cpp
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/IMGUI/imgui_widgets.cpp
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/IMGUI/examples/imgui_impl_glfw.cpp
	PRIVATE ${PROJECT_SOURCE_DIR}/external/src/IMGUI/examples/imgui_impl_opengl3.cpp
)
if (CUSTOM_ASSIMP STREQUAL "")
	add_dependencies(${Module} ASSIMP)
endif (CUSTOM_ASSIMP STREQUAL "")
if (CUSTOM_BULLET STREQUAL "")
	add_dependencies(${Module} BULLET)
endif (CUSTOM_BULLET STREQUAL "")
add_dependencies(${Module} GLAD)
if (CUSTOM_GLFW STREQUAL "")
	add_dependencies(${Module} GLFW)
endif (CUSTOM_GLFW STREQUAL "")
if (CUSTOM_GLM STREQUAL "")
	add_dependencies(${Module} GLM)
endif (CUSTOM_GLM STREQUAL "")
if (CUSTOM_IMGUI STREQUAL "")
	add_dependencies(${Module} IMGUI)
endif (CUSTOM_IMGUI STREQUAL "")
