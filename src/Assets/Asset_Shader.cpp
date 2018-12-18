#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Shader_Pkg.h"
#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include <fstream>


constexpr char* EXT_SHADER_VERTEX = ".vsh";
constexpr char* EXT_SHADER_FRAGMENT = ".fsh";
constexpr char* EXT_SHADER_BINARY = ".shader";
constexpr char* DIRECTORY_SHADER = "\\Shaders\\";

struct ShaderHeader { 
	GLenum format; 
	GLsizei length; 
};

Shared_Shader::Shared_Shader(Engine * engine, const std::string & filename, const bool & threaded) 
	: std::shared_ptr<Asset_Shader>(std::dynamic_pointer_cast<Asset_Shader>(engine->getAssetManager().shareAsset(typeid(Asset_Shader).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Asset_Shader>*)(this)) = std::make_shared<Asset_Shader>(filename);
		// Submit data to asset manager
		engine->getAssetManager().submitNewAsset(typeid(Asset_Shader).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Asset_Shader::initialize, get(), engine, (DIRECTORY_SHADER + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}


Asset_Shader::~Asset_Shader()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader::Asset_Shader(const std::string & filename) : Asset(filename) {}

void Asset_Shader::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	const std::string filename = getFileName();

	// Create Vertex Shader
	m_vertexShader.m_shaderText = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
	m_vertexShader.createGLShader(engine, filename);
	glAttachShader(m_glProgramID, m_vertexShader.m_shaderID);

	// Create Fragment Shader
	m_fragmentShader.m_shaderText = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
	m_fragmentShader.createGLShader(engine, filename);
	glAttachShader(m_glProgramID, m_fragmentShader.m_shaderID);

	glLinkProgram(m_glProgramID);
	glValidateProgram(m_glProgramID);

	// Detach the shaders now that the program is complete
	glDetachShader(m_glProgramID, m_vertexShader.m_shaderID);
	glDetachShader(m_glProgramID, m_fragmentShader.m_shaderID);
}

void Asset_Shader::initialize(Engine * engine, const std::string & relativePath)
{
	// Attempt to load cache, otherwise load manually
	m_glProgramID = glCreateProgram();

#ifdef NDEBUG
	if (!loadCachedBinary(engine, relativePath))
#endif
	{
		// Create Vertex and Fragment shaders
		bool success = initShaders(engine, relativePath);
		if (success) {
			glLinkProgram(m_glProgramID);
			success = validateProgram();
#ifdef NDEBUG
			if (success)
				saveCachedBinary(engine, relativePath);
#endif
		}
		// If we ever failed, initialize default shader
		if (!success) {
			const std::vector<GLchar> infoLog = getErrorLog();
			engine->getMessageManager().error("Asset_Shader \"" + m_filename + "\" failed to initialize. Reason: " + std::string(infoLog.data(), infoLog.size()));
			initializeDefault(engine);
		}
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

const GLint Asset_Shader::getProgramiv(const GLenum & pname) const
{
	GLint param;
	glGetProgramiv(m_glProgramID, pname, &param);
	return param;
}

const std::vector<GLchar> Asset_Shader::getErrorLog() const
{
	std::vector<GLchar> infoLog(getProgramiv(GL_INFO_LOG_LENGTH));
	glGetProgramInfoLog(m_glProgramID, (GLsizei)infoLog.size(), NULL, &infoLog[0]);
	return infoLog;
}

const bool Asset_Shader::loadCachedBinary(Engine * engine, const std::string & relativePath)
{
	if (Engine::File_Exists(relativePath + EXT_SHADER_BINARY)) {
		ShaderHeader header;
		std::ifstream file((Engine::Get_Current_Dir() + relativePath + EXT_SHADER_BINARY).c_str(), std::ios::binary);
		if (file.is_open()) {
			file.read(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
			std::vector<char> binary(header.length); 
			file.read(binary.data(), header.length);
			file.close();

			glProgramBinary(m_glProgramID, header.format, binary.data(), header.length);
			if (getProgramiv(GL_LINK_STATUS)) {
				glValidateProgram(m_glProgramID);
				return true;
			}
			const std::vector<GLchar> infoLog = getErrorLog();
			engine->getMessageManager().error("Asset_Shader \"" + m_filename + "\" failed to use binary cache. Reason:\n" + std::string(infoLog.data(), infoLog.size()));
			return false;
		}
		engine->getMessageManager().error("Asset_Shader \"" + m_filename + "\" failed to open binary cache.");
		return false;
	}
	// Safe, binary file simply doesn't exist. Don't error report.
	return false;
}

const bool Asset_Shader::saveCachedBinary(Engine * engine, const std::string & relativePath)
{
	glProgramParameteri(m_glProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
	ShaderHeader header = { 0,  getProgramiv(GL_PROGRAM_BINARY_LENGTH) };
	std::vector<char> binary(header.length);
	glGetProgramBinary(m_glProgramID, header.length, NULL, &header.format, binary.data());

	std::ofstream file((Engine::Get_Current_Dir() + relativePath + EXT_SHADER_BINARY).c_str(), std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
		file.write(binary.data(), header.length);
		file.close();
		return true;
	}
	engine->getMessageManager().error("Asset_Shader \"" + m_filename + "\" failed to write to binary cache.");
	return false;
}

const bool Asset_Shader::initShaders(Engine * engine, const std::string & relativePath) 
{
	const std::string filename = getFileName();

	if (!m_vertexShader.loadDocument(engine, relativePath + EXT_SHADER_VERTEX) ||
		!m_fragmentShader.loadDocument(engine, relativePath + EXT_SHADER_FRAGMENT))	
		return false;
	

	// Create Vertex Shader
	m_vertexShader.createGLShader(engine, filename);
	glAttachShader(m_glProgramID, m_vertexShader.m_shaderID);

	// Create Fragment Shader
	m_fragmentShader.createGLShader(engine, filename);
	glAttachShader(m_glProgramID, m_fragmentShader.m_shaderID);

	return true;
}

const bool Asset_Shader::validateProgram() 
{
	// Check Validation
	if (getProgramiv(GL_LINK_STATUS)) {
		glValidateProgram(m_glProgramID);

		// Detach the shaders now that the program is complete
		glDetachShader(m_glProgramID, m_vertexShader.m_shaderID);
		glDetachShader(m_glProgramID, m_fragmentShader.m_shaderID);

		return true;
	}
	return false;
}

ShaderObj::~ShaderObj() { glDeleteShader(m_shaderID); }

ShaderObj::ShaderObj(const GLenum & type) : m_type(type) {}

const GLint ShaderObj::getShaderiv(const GLenum & pname) const
{
	GLint param;
	glGetShaderiv(m_shaderID, pname, &param);
	return param;
}

const bool ShaderObj::loadDocument(Engine * engine, const std::string & filePath) 
{
	// Exit early if document not found or no text is found in the document
	if (!Text_IO::Import_Text(engine, filePath, m_shaderText) || m_shaderText == "")
		return false;

	// Update document, including any packages required
	size_t spot = m_shaderText.find("#package");
	while (spot != std::string::npos) {
		std::string directory = m_shaderText.substr(spot);

		const size_t qspot1 = directory.find("\"");
		const size_t qspot2 = directory.find("\"", qspot1 + 1);
		// find std::string quotes and remove them
		directory = directory.substr(qspot1 + 1, qspot2 - 1 - qspot1);

		Shared_Shader_Pkg package = Shared_Shader_Pkg(engine, directory, false);
		std::string left = m_shaderText.substr(0, spot);
		std::string right = m_shaderText.substr(spot + 1 + qspot2);
		m_shaderText = left + package->getPackageText() + right;
		spot = m_shaderText.find("#package");
	}

	// Success
	return true;
}

const bool ShaderObj::createGLShader(Engine * engine, const std::string & filename) 
{
	// Create shader object
	const char * source = m_shaderText.c_str();
	const GLint length = (GLint)m_shaderText.length();
	m_shaderID = glCreateShader(m_type);
	glShaderSource(m_shaderID, 1, &source, &length);
	glCompileShader(m_shaderID);

	// Validate shader object
	if (getShaderiv(GL_COMPILE_STATUS))
		return true;

	// Report any errors
	std::vector<GLchar> infoLog(getShaderiv(GL_INFO_LOG_LENGTH));
	glGetShaderInfoLog(m_shaderID, (GLsizei)infoLog.size(), NULL, &infoLog[0]);
	engine->getMessageManager().error("ShaderObj \"" + filename + "\" failed to compile. Reason:\n" + std::string(infoLog.data(), infoLog.size()));
	return false;
}