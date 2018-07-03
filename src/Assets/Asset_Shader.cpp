#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Shader_Pkg.h"
#include "Engine.h"
#include <fstream>

/** Read a file from disk.
 * @param	returnFile		the destination to load the text into
 * @param	fileDirectory	the file directory to load from
 * @return					returns true if file read successfull, false otherwise */
inline bool fetch_file_from_disk(string & returnFile, const string & fileDirectory)
{
	struct stat buffer;
	if (stat(fileDirectory.c_str(), &buffer))
		return false;

	ifstream file(fileDirectory);
	while (!file.eof()) {
		string temp;
		std::getline(file, temp);
		returnFile.append(temp + '\n');
	}

	return true;
}

/** Parse the shader, looking for any directives that require us to modify the document.
 * @param	engine			the engine being used
 * @param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Shared_Asset_Shader & userAsset)
{
	string *text[3] = { &userAsset->m_vertexText, &userAsset->m_fragmentText, &userAsset->m_geometryText };
	for (int x = 0; x < 3; ++x) {
		if (*text[x] == "") continue;
		string input = *text[x];
		// Find Package to include
		int spot = input.find("#package");
		while (spot != string::npos) {
			string directory = input.substr(spot);

			unsigned int qspot1 = directory.find("\"");
			unsigned int qspot2 = directory.find("\"", qspot1 + 1);
			// find string quotes and remove them
			directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

			Shared_Asset_Shader_Pkg package;
			engine->createAsset(package, directory, false);
			string left = input.substr(0, spot);
			string right = input.substr(spot + 1 + qspot2);
			input = left + package->getPackageText() + right;
			spot = input.find("#package");
		}
		unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
		*text[x] = input;
	}
}

/** Compile a single shader object.
 * @param	engine		the engine to be used
 * @param	filename	the shader filename
 * @param	ID			the shader ID to update
 * @param	source		the char array representing the document
 * @param	type		the shader type */
inline void compile_single_shader(Engine * engine, const string & filename, GLuint & ID, const char * source, const GLenum & type)
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
			engine->reportError(MessageManager::SHADER_INCOMPLETE, filename, string(InfoLog, 1024));
		}
	}
}

/** Compile all the shaders representing a shader program.
 * @param	engine		the engine to be used
 * @param	userAsset	the shader asset to compile */
inline void compile(Engine * engine, Shared_Asset_Shader & userAsset)
{
	compile_single_shader(engine, userAsset->getFileName(), userAsset->m_glVertexID, userAsset->m_vertexText.c_str(), GL_VERTEX_SHADER);
	compile_single_shader(engine, userAsset->getFileName(), userAsset->m_glFragmentID, userAsset->m_fragmentText.c_str(), GL_FRAGMENT_SHADER);
	compile_single_shader(engine, userAsset->getFileName(), userAsset->m_glGeometryID, userAsset->m_geometryText.c_str(), GL_GEOMETRY_SHADER);
}

/** Generate the shader program.
 * @param	userAsset	the shader asset to generate for */
void generate_program(Shared_Asset_Shader & userAsset)
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
inline void link_program(Engine * engine, Shared_Asset_Shader & userAsset)
{
	// Link and validate, retrieve any errors
	glLinkProgram(userAsset->m_glProgramID);
	GLint success;
	glGetProgramiv(userAsset->m_glProgramID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(userAsset->m_glProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		engine->reportError(MessageManager::PROGRAM_INCOMPLETE, userAsset->getFileName(), string(ErrorLog, 1024));
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

Asset_Shader::~Asset_Shader()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader::Asset_Shader(const string & filename) : Asset(filename)
{
	m_glProgramID = 0;
	m_glVertexID = 0;
	m_glFragmentID = 0;
	m_glGeometryID = 0;
	m_vertexText = "";
	m_fragmentText = "";
	m_geometryText = "";
}

void Asset_Shader::CreateDefault(Engine * engine, Shared_Asset_Shader & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultShader"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultShader");
	userAsset->m_vertexText = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
	userAsset->m_fragmentText = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}


void Asset_Shader::Create(Engine * engine, Shared_Asset_Shader & userAsset, const string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_SHADER + filename;
	bool found_vertex = File_Reader::FileExistsOnDisk(fullDirectory + EXT_SHADER_VERTEX);
	bool found_fragement = File_Reader::FileExistsOnDisk(fullDirectory + EXT_SHADER_FRAGMENT);
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

void Asset_Shader::Initialize(Engine * engine, Shared_Asset_Shader & userAsset, const string & fullDirectory)
{
	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	bool found_vertex = fetch_file_from_disk(userAsset->m_vertexText, fullDirectory + EXT_SHADER_VERTEX);
	bool found_fragement = fetch_file_from_disk(userAsset->m_fragmentText, fullDirectory + EXT_SHADER_FRAGMENT);
	bool found_geometry = fetch_file_from_disk(userAsset->m_geometryText, fullDirectory + EXT_SHADER_GEOMETRY);
	write_guard.unlock();
	write_guard.release();

	if (!found_vertex)
		engine->reportError(MessageManager::FILE_MISSING, userAsset->getFileName() + EXT_SHADER_VERTEX);
	if (!found_fragement)
		engine->reportError(MessageManager::FILE_MISSING, userAsset->getFileName() + EXT_SHADER_FRAGMENT);
	if (!(found_vertex + found_fragement + found_geometry)) {
		// handle this?
	}

	// parse
	parse(engine, userAsset);
}

void Asset_Shader::Finalize(Engine * engine, Shared_Asset_Shader & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_finalized = true;
	compile(engine, userAsset);
	generate_program(userAsset);
	link_program(engine, userAsset);

	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
	/* To Do: Finalize call here*/
}

void Asset_Shader::bind()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	glUseProgram(m_glProgramID);
}

void Asset_Shader::Release()
{
	glUseProgram(0);
}