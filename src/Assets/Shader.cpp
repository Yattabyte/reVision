#include "Assets/Shader.h"
#include "Assets/Shader_Pkg.h"
#include "Utilities/IO/Text_IO.h"
#include "Engine.h"
#include <filesystem>
#include <fstream>


constexpr char* EXT_SHADER_VERTEX = ".vsh";
constexpr char* EXT_SHADER_FRAGMENT = ".fsh";
constexpr char* EXT_SHADER_BINARY = ".shader";
constexpr char* DIRECTORY_SHADER = "\\Shaders\\";
constexpr char* DIRECTORY_SHADER_CACHE = R"(\cache\shaders\)";

struct ShaderHeader {
	GLenum format;
	GLsizei length;
};

Shared_Shader::Shared_Shader(Engine* engine, const std::string& filename, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Shader>*)(this)) = std::dynamic_pointer_cast<Shader>(
		engine->getManager_Assets().shareAsset(
			typeid(Shader).name(),
			filename,
			[engine, filename]() { return std::make_shared<Shader>(engine, filename); },
			threaded
		));
}

Shader::~Shader() noexcept
{
	if (existsYet())
		glDeleteProgram(m_glProgramID);
}

Shader::Shader(Engine* engine, const std::string& filename) noexcept : Asset(engine, filename) {}

void Shader::initialize() noexcept
{
	// Attempt to load cache, otherwise load manually
	m_glProgramID = glCreateProgram();

#ifdef NDEBUG
	//if (!loadCachedBinary(DIRECTORY_SHADER_CACHE + getFileName()))
#endif
	{
		// Create Vertex and Fragment shaders
		bool success = initShaders(DIRECTORY_SHADER + getFileName());
		if (success) {
			glLinkProgram(m_glProgramID);
			success = validateProgram();
#ifdef NDEBUG
			if (success)
				saveCachedBinary(DIRECTORY_SHADER_CACHE + getFileName());
#endif
		}
		// If we ever failed, initialize default shader
		if (!success) {
			const std::vector<GLchar> infoLog = getErrorLog();
			m_engine->getManager_Messages().error("Shader \"" + m_filename + "\" failed to initialize. Reason: " + std::string(infoLog.data(), infoLog.size()));

			// Create hard-coded alternative
			const std::string filename = getFileName();

			// Create Vertex Shader
			m_vertexShader.m_shaderText = "#version 430\n\nlayout(location = 0) in vec3 vertex;\n\nvoid main()\n{\n\tgl_Position = vec4(vertex, 1.0);\n}";
			m_vertexShader.createGLShader(m_engine, filename);
			glAttachShader(m_glProgramID, m_vertexShader.m_shaderID);

			// Create Fragment Shader
			m_fragmentShader.m_shaderText = "#version 430\n\nlayout (location = 0) out vec4 fragColor;\n\nvoid main()\n{\n\tfragColor = vec4(1.0f);\n}";
			m_fragmentShader.createGLShader(m_engine, filename);
			glAttachShader(m_glProgramID, m_fragmentShader.m_shaderID);

			glLinkProgram(m_glProgramID);
			glValidateProgram(m_glProgramID);

			// Detach the shaders now that the program is complete
			glDetachShader(m_glProgramID, m_vertexShader.m_shaderID);
			glDetachShader(m_glProgramID, m_fragmentShader.m_shaderID);
		}
	}

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize();
}

void Shader::bind() noexcept
{
	glUseProgram(m_glProgramID);
}

void Shader::Release() noexcept
{
	glUseProgram(0);
}

const GLint Shader::getProgramiv(const GLenum& pname) const noexcept
{
	GLint param;
	glGetProgramiv(m_glProgramID, pname, &param);
	return param;
}

std::vector<GLchar> Shader::getErrorLog() const noexcept
{
	const auto size = getProgramiv(GL_INFO_LOG_LENGTH);
	std::vector<GLchar> infoLog(size);
	if (size)
		glGetProgramInfoLog(m_glProgramID, (GLsizei)infoLog.size(), nullptr, &infoLog[0]);
	return infoLog;
}

const bool Shader::loadCachedBinary(const std::string& relativePath) noexcept
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
			if (getProgramiv(GL_LINK_STATUS) != 0) {
				glValidateProgram(m_glProgramID);
				return true;
			}
			const std::vector<GLchar> infoLog = getErrorLog();
			m_engine->getManager_Messages().error("Shader \"" + m_filename + "\" failed to use binary cache. Reason:\n" + std::string(infoLog.data(), infoLog.size()));
			return false;
		}
		m_engine->getManager_Messages().error("Shader \"" + m_filename + "\" failed to open binary cache.");
		return false;
	}
	// Safe, binary file simply doesn't exist. Don't error report.
	return false;
}

const bool Shader::saveCachedBinary(const std::string& relativePath) noexcept
{
	glProgramParameteri(m_glProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
	ShaderHeader header = { 0,  getProgramiv(GL_PROGRAM_BINARY_LENGTH) };
	std::vector<char> binary(header.length);
	glGetProgramBinary(m_glProgramID, header.length, nullptr, &header.format, binary.data());

	const auto fullPath = Engine::Get_Current_Dir() + relativePath + EXT_SHADER_BINARY;
	std::filesystem::create_directories(std::filesystem::path(fullPath).parent_path());
	std::ofstream file(fullPath, std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<char*>(&header), sizeof(ShaderHeader));
		file.write(binary.data(), header.length);
		file.close();
		return true;
	}
	m_engine->getManager_Messages().error("Shader \"" + m_filename + "\" failed to write to binary cache.");
	return false;
}

bool Shader::initShaders(const std::string& relativePath) noexcept
{
	const std::string filename = getFileName();

	if (!m_vertexShader.loadDocument(m_engine, relativePath + EXT_SHADER_VERTEX) ||
		!m_fragmentShader.loadDocument(m_engine, relativePath + EXT_SHADER_FRAGMENT))
		return false;


	// Create Vertex Shader
	m_vertexShader.createGLShader(m_engine, filename);
	glAttachShader(m_glProgramID, m_vertexShader.m_shaderID);

	// Create Fragment Shader
	m_fragmentShader.createGLShader(m_engine, filename);
	glAttachShader(m_glProgramID, m_fragmentShader.m_shaderID);

	return true;
}

const bool Shader::validateProgram() noexcept
{
	// Check Validation
	if (getProgramiv(GL_LINK_STATUS) != 0) {
		glValidateProgram(m_glProgramID);

		// Detach the shaders now that the program is complete
		glDetachShader(m_glProgramID, m_vertexShader.m_shaderID);
		glDetachShader(m_glProgramID, m_fragmentShader.m_shaderID);

		return true;
	}
	return false;
}

ShaderObj::~ShaderObj() noexcept { glDeleteShader(m_shaderID); }

ShaderObj::ShaderObj(const GLenum& type) noexcept : m_type(type) {}

GLint ShaderObj::getShaderiv(const GLenum& parameterName) const noexcept
{
	GLint param;
	glGetShaderiv(m_shaderID, parameterName, &param);
	return param;
}

bool ShaderObj::loadDocument(Engine* engine, const std::string& filePath) noexcept
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

bool ShaderObj::createGLShader(Engine* engine, const std::string& filename) noexcept
{
	// Create shader object
	const char* source = m_shaderText.c_str();
	const auto length = (GLint)m_shaderText.length();
	m_shaderID = glCreateShader(m_type);
	glShaderSource(m_shaderID, 1, &source, &length);
	glCompileShader(m_shaderID);

	// Validate shader object
	if (getShaderiv(GL_COMPILE_STATUS) != 0)
		return true;

	// Report any errors
	std::vector<GLchar> infoLog(getShaderiv(GL_INFO_LOG_LENGTH));
	glGetShaderInfoLog(m_shaderID, (GLsizei)infoLog.size(), nullptr, &infoLog[0]);
	engine->getManager_Messages().error("ShaderObj \"" + filename + "\" failed to compile. Reason:\n" + std::string(infoLog.data(), infoLog.size()));
	return false;
}