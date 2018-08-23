#include "Assets\Asset_Primitive.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"

#define EXT_PRIMITIVE ".obj"
#define DIRECTORY_PRIMITIVE Engine::Get_Current_Dir() + "\\Primitives\\"
#define ABS_DIRECTORY_PRIMITIVE(filename) DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE


Asset_Primitive::~Asset_Primitive()
{
	if (existsYet())
		glDeleteBuffers(2, m_buffers);
}

Asset_Primitive::Asset_Primitive(const std::string & filename) : Asset(filename)
{
	for each (auto &buffer in m_buffers)
		buffer = -1;
}

Shared_Asset_Primitive Asset_Primitive::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Primitive>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Primitive>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_PRIMITIVE(filename);
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory)) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Primitive::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_dataVertex = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	m_dataUV = std::vector<glm::vec2>{ glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(0, 1) };
}

void Asset_Primitive::initialize(Engine * engine, const std::string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_primitive, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Primitive");
		initializeDefault(engine);
		return;
	}

	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	m_dataVertex = dataContainer.vertices;
	m_dataUV = dataContainer.texCoords;
}

void Asset_Primitive::finalize(Engine * engine)
{
	// Create buffers
	{
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		glCreateBuffers(2, m_buffers);
	}
	// Load Buffers
	{
		std::shared_lock<std::shared_mutex> read_guard(m_mutex);
		glNamedBufferStorage(m_buffers[0], m_dataVertex.size() * sizeof(glm::vec3), &m_dataVertex[0][0], GL_CLIENT_STORAGE_BIT);
		glNamedBufferStorage(m_buffers[1], m_dataVertex.size() * sizeof(glm::vec2), &m_dataUV[0][0], GL_CLIENT_STORAGE_BIT);		
	}
	Asset::finalize(engine);
}

const GLuint Asset_Primitive::Generate_VAO()
{
	GLuint vaoID = 0;

	glCreateVertexArrays(1, &vaoID);
	glEnableVertexArrayAttrib(vaoID, 0);
	glEnableVertexArrayAttrib(vaoID, 1);

	return vaoID;
}

void Asset_Primitive::updateVAO(const GLuint & vaoID)
{
	std::shared_lock<std::shared_mutex> guard(m_mutex);
	
	glVertexArrayAttribFormat(vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(vaoID, 1, 2, GL_FLOAT, GL_FALSE, 0);

	glVertexArrayVertexBuffer(vaoID, 0, m_buffers[0], 0, 12);
	glVertexArrayVertexBuffer(vaoID, 1, m_buffers[1], 0, 8);

	glVertexArrayAttribBinding(vaoID, 0, 0);
	glVertexArrayAttribBinding(vaoID, 1, 1);
}

size_t Asset_Primitive::getSize()
{
	std::shared_lock<std::shared_mutex> guard(m_mutex);
	return m_dataVertex.size();
}
