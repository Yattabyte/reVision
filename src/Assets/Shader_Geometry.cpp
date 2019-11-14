#include "Assets/Shader_Geometry.h"
#include "Assets/Shader_Pkg.h"
#include "Utilities/IO/Text_IO.h"
#include "Engine.h"


constexpr char* EXT_SHADER_GEOMETRY = ".gsh";
constexpr char* DIRECTORY_SHADER = "\\Shaders\\";

Shared_Shader_Geometry::Shared_Shader_Geometry(Engine* engine, const std::string& filename, const bool& threaded) noexcept
{
	(*(std::shared_ptr<Shader_Geometry>*)(this)) = std::dynamic_pointer_cast<Shader_Geometry>(
		engine->getManager_Assets().shareAsset(
			typeid(Shader_Geometry).name(),
			filename,
			[engine, filename]() { return std::make_shared<Shader_Geometry>(engine, filename); },
			threaded
		));
}

Shader_Geometry::~Shader_Geometry() noexcept
{
	if (existsYet())
		glDeleteProgram(m_glProgramID);
}

Shader_Geometry::Shader_Geometry(Engine* engine, const std::string& filename) noexcept : Shader(engine, filename) {}

void Shader_Geometry::initialize() noexcept
{
	// Attempt to load cache, otherwise load manually
	m_glProgramID = glCreateProgram();
	if (!loadCachedBinary(DIRECTORY_SHADER + getFileName())) {
		bool success = false;
		// Create Geometry shader
		if (initShaders(DIRECTORY_SHADER + getFileName())) {
			glLinkProgram(m_glProgramID);
			if (validateProgram()) {
				glDetachShader(m_glProgramID, m_geometryShader.m_shaderID);
				saveCachedBinary(DIRECTORY_SHADER + getFileName());
				success = true;
			}
		}
		if (!success) {
			// Initialize default
			const std::vector<GLchar> infoLog = getErrorLog();
			m_engine->getManager_Messages().error("Shader_Geometry \"" + m_filename + "\" failed to initialize. Reason: \n" + std::string(infoLog.data(), infoLog.size()));
		}
	}

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize();
}

bool Shader_Geometry::initShaders(const std::string& relativePath)
{
	const std::string filename = getFileName();

	if (!Shader::initShaders(relativePath) ||
		!m_geometryShader.loadDocument(m_engine, relativePath + EXT_SHADER_GEOMETRY))
		return false;

	// Create Geometry Shader
	m_geometryShader.createGLShader(m_engine, filename);
	glAttachShader(m_glProgramID, m_geometryShader.m_shaderID);

	return true;
}