#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"


constexpr char* EXT_SHADER_VERTEX = ".vsh";
constexpr char* EXT_SHADER_FRAGMENT = ".fsh";
constexpr char* EXT_SHADER_BINARY = ".shader";
constexpr char* DIRECTORY_SHADER = "\\Shaders\\";

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
			glGetShaderInfoLog(ID, (GLsizei)infoLog.size(), NULL, &infoLog[0]);
			engine->getMessageManager().error(MessageManager::SHADER_INCOMPLETE, filename, std::string(infoLog.data(), infoLog.size()));
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
		glGetProgramInfoLog(userAsset.m_glProgramID, (GLsizei)infoLog.size(), NULL, &infoLog[0]);
		engine->getMessageManager().error(MessageManager::PROGRAM_INCOMPLETE, userAsset.getFileName(), std::string(infoLog.data(), infoLog.size()));
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
@param	userAsset	the shader asset to link for 
@return				true if successful, false otherwise */
inline bool use_binary(Engine * engine, Asset_Shader & userAsset)
{
	userAsset.m_glProgramID = glCreateProgram();
	glProgramBinary(userAsset.m_glProgramID, userAsset.m_binaryFormat, userAsset.m_binary.data(), userAsset.m_binaryLength);
	GLint success;
	glGetProgramiv(userAsset.m_glProgramID, GL_LINK_STATUS, &success);
	if (success == 0) {
		GLint infoLogLength;
		glGetProgramiv(userAsset.m_glProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(userAsset.m_glProgramID, (GLsizei)infoLog.size(), NULL, &infoLog[0]);
		engine->getMessageManager().error(MessageManager::PROGRAM_INCOMPLETE, userAsset.getFileName(), std::string(infoLog.data(), infoLog.size()));
	}
	glValidateProgram(userAsset.m_glProgramID);
	return (bool)success;
}

/** Save the program binary to file
@param	engine		the engine to be used
@param	userAsset	the shader asset to link for */
inline void save_binary(Engine * engine, Asset_Shader & userAsset)
{
	ShaderHeader header;
	glProgramParameteri(userAsset.m_glProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
	glGetProgramiv(userAsset.m_glProgramID, GL_PROGRAM_BINARY_LENGTH, &header.length);
	userAsset.m_binary.resize(header.length);
	glGetProgramBinary(userAsset.m_glProgramID, header.length, NULL, &header.format, userAsset.m_binary.data());

	std::ofstream file((Engine::Get_Current_Dir() + DIRECTORY_SHADER + userAsset.getFileName() + EXT_SHADER_BINARY).c_str(), std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
		file.write(userAsset.m_binary.data(), header.length);
		file.close();
	}
	userAsset.m_binaryFormat = header.format;
	userAsset.m_binaryLength = header.length;	
}

Asset_Shader::~Asset_Shader()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader::Asset_Shader(const std::string & filename) : Asset(filename) {}

Shared_Asset_Shader Asset_Shader::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Shader>(
		filename,
		DIRECTORY_SHADER,
		"",
		&initialize,
		engine,
		threaded
	);
}

void Asset_Shader::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_vertexText = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
	m_fragmentText = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
}

void Asset_Shader::initialize(Engine * engine, const std::string & relativePath)
{
	const bool found_vertex = Text_IO::Import_Text(engine, relativePath + EXT_SHADER_VERTEX, m_vertexText);
	const bool found_fragement = Text_IO::Import_Text(engine, relativePath + EXT_SHADER_FRAGMENT, m_fragmentText);
	const bool found_shader_binary = Engine::File_Exists(relativePath + EXT_SHADER_BINARY);
	
	if (!(found_vertex && found_fragement)) {
		engine->getMessageManager().error(MessageManager::ASSET_FAILED, "Asset_Shader");
		initializeDefault(engine);
	}

	// Try to use the cached shader
	bool binarySuccess = false;
	if (found_shader_binary) {
		ShaderHeader header;
		std::ifstream file((Engine::Get_Current_Dir() + relativePath + EXT_SHADER_BINARY).c_str(), std::ios::binary);
		if (file.is_open()) {
			file.read(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
			m_binary.resize(header.length);
			file.read(m_binary.data(), header.length);
			m_binaryFormat = header.format;
			m_binaryLength = header.length;
			file.close();
			binarySuccess = use_binary(engine, *this);
		}
	}
	if (!binarySuccess) {
		parse(engine, *this);
		compile(engine, *this);
		generate_program(*this);
		link_program(engine, *this);
		save_binary(engine, *this);
		cleanup_program(*this);
	}

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
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