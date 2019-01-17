#include "Assets/Asset_Shader_Geometry.h"
#include "Assets/Asset_Shader_Pkg.h"
#include "Utilities/IO/Text_IO.h"
#include "Engine.h"


constexpr char* EXT_SHADER_GEOMETRY = ".gsh";
constexpr char* DIRECTORY_SHADER = "\\Shaders\\";

Shared_Shader_Geometry::Shared_Shader_Geometry(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Shader_Geometry>(std::dynamic_pointer_cast<Asset_Shader_Geometry>(engine->getManager_Assets().shareAsset(typeid(Asset_Shader_Geometry).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Asset_Shader_Geometry>*)(this)) = std::make_shared<Asset_Shader_Geometry>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Asset_Shader_Geometry).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Asset_Shader_Geometry::initialize, get(), engine, (DIRECTORY_SHADER + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Asset_Shader_Geometry::~Asset_Shader_Geometry()
{
	if (existsYet()) 
		glDeleteProgram(m_glProgramID);
}

Asset_Shader_Geometry::Asset_Shader_Geometry(const std::string & filename) : Asset_Shader(filename) {}

void Asset_Shader_Geometry::initializeDefault()
{	
}

void Asset_Shader_Geometry::initialize(Engine * engine, const std::string & relativePath)
{
	// Attempt to load cache, otherwise load manually
	m_glProgramID = glCreateProgram();
	bool success = false;
	if (!loadCachedBinary(engine, relativePath)) {
		// Create Geometry shader
		if (initShaders(engine, relativePath)) {
			glLinkProgram(m_glProgramID);
			if (validateProgram()) {
				glDetachShader(m_glProgramID, m_geometryShader.m_shaderID);
				saveCachedBinary(engine, relativePath);
				success = true;
			}
		}
		if (!success) {
			// Initialize default
			const std::vector<GLchar> infoLog = getErrorLog();
			engine->getManager_Messages().error("Asset_Shader_Geometry \"" + m_filename + "\" failed to initialize. Reason: \n" + std::string(infoLog.data(), infoLog.size()));
			initializeDefault();
		}
	}

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize(engine);
}

const bool Asset_Shader_Geometry::initShaders(Engine * engine, const std::string & relativePath)
{
	const std::string filename = getFileName();

	if (!Asset_Shader::initShaders(engine, relativePath) ||
		!m_geometryShader.loadDocument(engine, relativePath + EXT_SHADER_GEOMETRY))
		return false;

	// Create Geometry Shader
	m_geometryShader.createGLShader(engine, filename);
	glAttachShader(m_glProgramID, m_geometryShader.m_shaderID);

	return true;
}