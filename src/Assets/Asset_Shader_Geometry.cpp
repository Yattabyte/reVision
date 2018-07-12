#include "Assets\Asset_Shader_Geometry.h"
#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#define EXT_SHADER_VERTEX ".vsh"
#define EXT_SHADER_FRAGMENT ".fsh"
#define EXT_SHADER_GEOMETRY ".gsh"
#define DIRECTORY_SHADER Engine::Get_Current_Dir() + "\\Shaders\\"


/** Parse the shader, looking for any directives that require us to modify the document.
 * @param	engine			the engine being used
 * @param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Shared_Asset_Shader_Geometry & userAsset)
{
	std::string *text[3] = { &userAsset->m_vertexText, &userAsset->m_fragmentText, &userAsset->m_geometryText };
	for (int x = 0; x < 3; ++x) {
		if (*text[x] == "") continue;
		std::string input = *text[x];
		// Find Package to include
		int spot = input.find("#package");
		while (spot != std::string::npos) {
			std::string directory = input.substr(spot);

			unsigned int qspot1 = directory.find("\"");
			unsigned int qspot2 = directory.find("\"", qspot1 + 1);
			// find std::string quotes and remove them
			directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

			Shared_Asset_Shader_Pkg package;
			engine->createAsset(package, directory, false);
			std::string left = input.substr(0, spot);
			std::string right = input.substr(spot + 1 + qspot2);
			input = left + package->getPackageText() + right;
			spot = input.find("#package");
		}
		std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
		*text[x] = input;
	}
}

/** Compile a single shader object.
 * @param	engine		the engine to be used
 * @param	filename	the shader filename
 * @param	ID			the shader ID to update
 * @param	source		the char array representing the document
 * @param	type		the shader type */
inline void compile_single_shader(Engine * engine, const std::string & filename, GLuint & ID, const char * source, const GLenum & type)
{
	if (strlen(source) > 0) {
		ID = glCreateShader(type);
		glShaderSource(ID, 1, &source, NULL);
		glCompileShader(ID);

		GLint success;
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(ID, sizeof(InfoLog), NULL, InfoLog);
			engine->reportError(MessageManager::SHADER_INCOMPLETE, filename, std::string(InfoLog, 1024));
		}
	}
}

/** Compile all the shaders representing a shader program.
 * @param	engine		the engine to be used
 * @param	userAsset	the shader asset to compile */
inline void compile(Engine * engine, Shared_Asset_Shader_Geometry & userAsset)
{
	compile_single_shader(engine, userAsset->getFileName(), userAsset->m_glVertexID, userAsset->m_vertexText.c_str(), GL_VERTEX_SHADER);
	compile_single_shader(engine, userAsset->getFileName(), userAsset->m_glFragmentID, userAsset->m_fragmentText.c_str(), GL_FRAGMENT_SHADER);
	compile_single_shader(engine, userAsset->getFileName(), userAsset->m_glGeometryID, userAsset->m_geometryText.c_str(), GL_GEOMETRY_SHADER);
}

/** Generate the shader program.
 * @param	userAsset	the shader asset to generate for */
void generate_program(Shared_Asset_Shader_Geometry & userAsset)
{
	userAsset->m_glProgramID = glCreateProgram();

	if (userAsset->m_glVertexID != 0)
		glAttachShader(userAsset->m_glProgramID, userAsset->m_glVertexID);
	if (userAsset->m_glFragmentID != 0)
		glAttachShader(userAsset->m_glProgramID, userAsset->m_glFragmentID);
	if (userAsset->m_glGeometryID != 0)
		glAttachShader(userAsset->m_glProgramID, userAsset->m_glGeometryID);
}

/** Link the shader program.
 * @param	engine		the engine to be used
 * @param	userAsset	the shader asset to link for */
inline void link_program(Engine * engine, Shared_Asset_Shader_Geometry & userAsset)
{
	// Link and validate, retrieve any errors
	glLinkProgram(userAsset->m_glProgramID);
	GLint success;
	glGetProgramiv(userAsset->m_glProgramID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(userAsset->m_glProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		engine->reportError(MessageManager::PROGRAM_INCOMPLETE, userAsset->getFileName(), std::string(ErrorLog, 1024));
	}
	glValidateProgram(userAsset->m_glProgramID);

	// Delete shader objects, they are already compiled and attached
	if (userAsset->m_glVertexID != 0)
		glDeleteShader(userAsset->m_glVertexID);
	if (userAsset->m_glFragmentID != 0)
		glDeleteShader(userAsset->m_glFragmentID);
	if (userAsset->m_glGeometryID != 0)
		glDeleteShader(userAsset->m_glGeometryID);
}

Asset_Shader_Geometry::~Asset_Shader_Geometry()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader_Geometry::Asset_Shader_Geometry(const std::string & filename) : Asset_Shader(filename)
{
	m_glProgramID = 0;
	m_glVertexID = 0;
	m_glFragmentID = 0;
	m_glGeometryID = 0;
	m_vertexText = "";
	m_fragmentText = "";
	m_geometryText = "";
}

void Asset_Shader_Geometry::CreateDefault(Engine * engine, Shared_Asset_Shader_Geometry & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultShader"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultShader");
	userAsset->m_vertexText = "#version 430\n\nlayout(location = 0) in glm::vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = glm::vec4(vertex, 1.0);\n}";
	userAsset->m_fragmentText = "#version 430\n\nlayout (location = 0) out glm::vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = glm::vec4(1.0f);\n}";
	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}


void Asset_Shader_Geometry::Create(Engine * engine, Shared_Asset_Shader_Geometry & userAsset, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_SHADER + filename;
	bool found_vertex = Engine::File_Exists(fullDirectory + EXT_SHADER_VERTEX);
	bool found_fragement = Engine::File_Exists(fullDirectory + EXT_SHADER_FRAGMENT);
	if (!found_vertex)
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory + EXT_SHADER_VERTEX);
	if (!found_fragement)
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory + EXT_SHADER_FRAGMENT);
	if (!(found_vertex && found_fragement)) {
		CreateDefault(engine, userAsset);
		return;
	}

	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[engine, &userAsset, fullDirectory]() mutable { Initialize(engine, userAsset, fullDirectory); },
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
		/* Constructor Arguments. */
		filename
	);
}

void Asset_Shader_Geometry::Initialize(Engine * engine, Shared_Asset_Shader_Geometry & userAsset, const std::string & fullDirectory)
{
	std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
	const bool found_vertex = Text_IO::Import_Text(engine, fullDirectory + EXT_SHADER_VERTEX, userAsset->m_vertexText);
	const bool found_fragement = Text_IO::Import_Text(engine, fullDirectory + EXT_SHADER_FRAGMENT, userAsset->m_fragmentText);
	const bool found_geometry = Text_IO::Import_Text(engine, fullDirectory + EXT_SHADER_GEOMETRY, userAsset->m_geometryText);
	write_guard.unlock();
	write_guard.release();

	if (!(found_vertex && found_fragement)) {
		CreateDefault(engine, userAsset);
		return;
	}

	// parse
	parse(engine, userAsset);
}

void Asset_Shader_Geometry::Finalize(Engine * engine, Shared_Asset_Shader_Geometry & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();	
	userAsset->finalize();

	// Create Shader Program
	{
		std::unique_lock<std::shared_mutex> write_guard(userAsset->m_mutex);
		compile(engine, userAsset);
		generate_program(userAsset);
		link_program(engine, userAsset);
	}

	// Notify Completion
	{
		std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
		for each (auto qwe in userAsset->m_callbacks)
			assetManager.submitNotifyee(qwe.second); 
	}
}