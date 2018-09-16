#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"

#define EXT_SHADER_VERTEX ".vsh"
#define EXT_SHADER_FRAGMENT ".fsh"
#define EXT_SHADER_BINARY ".shader"
#define DIRECTORY_SHADER Engine::Get_Current_Dir() + "\\Shaders\\"


struct ShaderHeader { 
	GLenum format;	
	GLsizei length; 
};

/** Parse the shader, looking for any directives that require us to modify the document.
@param	engine			the engine being used
@param	userAsset		the asset we are loading from */
inline void parse(Engine * engine, Asset_Shader & userAsset)
{
	std::string *text[2] = { &userAsset.m_vertexText, &userAsset.m_fragmentText };
	for (int x = 0; x < 2; ++x) {
		std::string input;
		if (*text[x] == "") continue;
		input = *text[x];
		// Find Package to include
		size_t spot = input.find("#package");
		while (spot != std::string::npos) {
			std::string directory = input.substr(spot);

			size_t qspot1 = directory.find("\"");
			size_t qspot2 = directory.find("\"", qspot1 + 1);
			// find std::string quotes and remove them
			directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

			Shared_Asset_Shader_Pkg package = Asset_Shader_Pkg::Create(engine, directory, false);
			std::string left = input.substr(0, spot);
			std::string right = input.substr(spot + 1 + qspot2);
			input = left + package->getPackageText() + right;
			spot = input.find("#package");
		}			
		*text[x] = input;
	}
}

/** Compile a single shader object.
@param	engine		the engine to be used
@param	filename	the shader filename
@param	ID			the shader ID to update
@param	source		the char array representing the document
@param	type		the shader type */
inline void compile_single_shader(Engine * engine, const std::string & filename, GLuint & ID, const char * source, const GLenum & type)
{
	if (strlen(source) > 0) {
		ID = glCreateShader(type);
		glShaderSource(ID, 1, &source, NULL);
		glCompileShader(ID);

		GLint success;
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint infoLogLength;
			glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<GLchar> infoLog(infoLogLength);
			glGetShaderInfoLog(ID, infoLog.size(), NULL, &infoLog[0]);
			engine->reportError(MessageManager::SHADER_INCOMPLETE, filename, std::string(infoLog.data(), infoLog.size()));
		}
	}
}

/** Compile all the shaders representing a shader program.
@param	engine		the engine to be used
@param	userAsset	the shader asset to compile */
inline void compile(Engine * engine, Asset_Shader & userAsset)
{
	compile_single_shader(engine, userAsset.getFileName(), userAsset.m_glVertexID, userAsset.m_vertexText.c_str(), GL_VERTEX_SHADER);
	compile_single_shader(engine, userAsset.getFileName(), userAsset.m_glFragmentID, userAsset.m_fragmentText.c_str(), GL_FRAGMENT_SHADER);
}

/** Generate the shader program.
@param	userAsset	the shader asset to generate for */
void generate_program(Asset_Shader & userAsset)
{
	userAsset.m_glProgramID = glCreateProgram();

	if (userAsset.m_glVertexID != 0)
		glAttachShader(userAsset.m_glProgramID, userAsset.m_glVertexID);
	if (userAsset.m_glFragmentID != 0)
		glAttachShader(userAsset.m_glProgramID, userAsset.m_glFragmentID);
}

#include <fstream>

/** Link the shader program.
@param	engine		the engine to be used
@param	userAsset	the shader asset to link for */
inline void link_program(Engine * engine, Asset_Shader & userAsset)
{
	// Link and validate, retrieve any errors
	glLinkProgram(userAsset.m_glProgramID);
	GLint success;
	glGetProgramiv(userAsset.m_glProgramID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLint infoLogLength;
		glGetProgramiv(userAsset.m_glProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(userAsset.m_glProgramID, infoLog.size(), NULL, &infoLog[0]);
		engine->reportError(MessageManager::PROGRAM_INCOMPLETE, userAsset.getFileName(), std::string(infoLog.data(), infoLog.size()));
	}	
	glValidateProgram(userAsset.m_glProgramID);
}

/** Cleanup residual program files.
@param	userAsset	the shader asset to link for */
inline void cleanup_program(Asset_Shader & userAsset) {
	// Delete shader objects, they are already compiled and attached
	if (userAsset.m_glVertexID != 0) {
		glDetachShader(userAsset.m_glProgramID, userAsset.m_glVertexID);
		glDeleteShader(userAsset.m_glVertexID);
	}
	if (userAsset.m_glFragmentID != 0) {
		glDetachShader(userAsset.m_glProgramID, userAsset.m_glFragmentID);
		glDeleteShader(userAsset.m_glFragmentID);
	}
}

/** Save the program binary to file
@param	engine		the engine to be used
@param	userAsset	the shader asset to link for */
inline void use_binary(Engine * engine, Asset_Shader & userAsset)
{
	if (userAsset.m_hasBinary) {
		userAsset.m_glProgramID = glCreateProgram(); 
		glProgramBinary(userAsset.m_glProgramID, userAsset.m_binaryFormat, userAsset.m_binary.data(), userAsset.m_binaryLength);
		GLint success;
		glGetProgramiv(userAsset.m_glProgramID, GL_LINK_STATUS, &success);
		if (success == 0) {
			GLint infoLogLength;
			glGetProgramiv(userAsset.m_glProgramID, GL_INFO_LOG_LENGTH, &infoLogLength); 
			std::vector<GLchar> infoLog(infoLogLength);
			glGetProgramInfoLog(userAsset.m_glProgramID, infoLog.size(), NULL, &infoLog[0]);
			engine->reportError(MessageManager::PROGRAM_INCOMPLETE, userAsset.getFileName(), std::string(infoLog.data(),infoLog.size()));
		}
		glValidateProgram(userAsset.m_glProgramID);
	}
}

/** Save the program binary to file
@param	engine		the engine to be used
@param	userAsset	the shader asset to link for */
inline void save_binary(Engine * engine, Asset_Shader & userAsset)
{
	if (!userAsset.m_hasBinary) {
		ShaderHeader header;
		glProgramParameteri(userAsset.m_glProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		glGetProgramiv(userAsset.m_glProgramID, GL_PROGRAM_BINARY_LENGTH, &header.length);
		userAsset.m_binary.resize(header.length);
		glGetProgramBinary(userAsset.m_glProgramID, header.length, NULL, &header.format, userAsset.m_binary.data());

		std::ofstream file((DIRECTORY_SHADER + userAsset.getFileName() + EXT_SHADER_BINARY).c_str(), std::ios::binary);
		if (file.is_open()) {			
			file.write(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
			file.write(userAsset.m_binary.data(), header.length);
			file.close();
		}
		userAsset.m_binaryFormat = header.format;
		userAsset.m_binaryLength = header.length;
	}
}

Asset_Shader::~Asset_Shader()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader::Asset_Shader(const std::string & filename) : Asset(filename) {}

Shared_Asset_Shader Asset_Shader::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Shader>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Shader>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_SHADER + filename;
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		bool found_vertex = Engine::File_Exists(fullDirectory + EXT_SHADER_VERTEX);
		bool found_fragement = Engine::File_Exists(fullDirectory + EXT_SHADER_FRAGMENT);
		if (!found_vertex)
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory + EXT_SHADER_VERTEX);
		if (!found_fragement)
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory + EXT_SHADER_FRAGMENT);
		if (!(found_vertex && found_fragement))
			initFunc = std::bind(&initializeDefault, &assetRef, engine);

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Shader::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_vertexText = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
	m_fragmentText = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
}

void Asset_Shader::initialize(Engine * engine, const std::string & fullDirectory)
{
	const bool found_vertex = Text_IO::Import_Text(engine, fullDirectory + EXT_SHADER_VERTEX, m_vertexText);
	const bool found_fragement = Text_IO::Import_Text(engine, fullDirectory + EXT_SHADER_FRAGMENT, m_fragmentText);
	const bool found_shader_binary = Engine::File_Exists(fullDirectory + EXT_SHADER_BINARY);
	
	if (!(found_vertex && found_fragement)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Shader");
		initializeDefault(engine);
		return;
	}

	// Try to use the cached shader
	if (found_shader_binary) {
		ShaderHeader header;
		std::ifstream file((fullDirectory + EXT_SHADER_BINARY).c_str(), std::ios::binary);
		if (file.is_open()) {
			file.read(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
			m_binary.resize(header.length);
			file.read(m_binary.data(), header.length);
			m_binaryFormat = header.format;
			m_binaryLength = header.length;
			m_hasBinary = true;
			file.close();
		}
	}
	else
		parse(engine, *this);
}

void Asset_Shader::finalize(Engine * engine)
{
	// Create Shader Program
	if (m_hasBinary) 
		use_binary(engine, *this);	
	else {
		compile(engine, *this);
		generate_program(*this);
		link_program(engine, *this);
		save_binary(engine, *this);
		cleanup_program(*this);
	}
	
	// Finalize
	Asset::finalize(engine);
}

void Asset_Shader::bind()
{
	glUseProgram(m_glProgramID);
}

void Asset_Shader::Release()
{
	glUseProgram(0);
}