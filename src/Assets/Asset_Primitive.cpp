#include "Assets\Asset_Primitive.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"

#define EXT_PRIMITIVE ".obj"
#define DIRECTORY_PRIMITIVE Engine::Get_Current_Dir() + "\\Primitives\\"
#define ABS_DIRECTORY_PRIMITIVE(filename) DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE


Asset_Primitive::~Asset_Primitive()
{
	if (existsYet())
		glDeleteBuffers(1, &m_uboID);
}

Asset_Primitive::Asset_Primitive(const std::string & filename) : Asset(filename) 
{
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glEnableVertexArrayAttrib(m_vaoID, 1);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribBinding(m_vaoID, 1, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vaoID, 1, 2, GL_FLOAT, GL_FALSE, 12);
	glCreateBuffers(1, &m_uboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_uboID, 0, sizeof(Single_Primitive_Vertex));
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
	m_data = { 
		{ glm::vec3(-1, -1, 0), glm::vec2(0, 0) }, { glm::vec3(1, -1, 0), glm::vec2(1, 0) }, { glm::vec3(1, 1, 0), glm::vec2(1, 1) },
		{ glm::vec3(-1, -1, 0), glm::vec2(0, 0) }, {glm::vec3(1, 1, 0), glm::vec2(1, 1) }, { glm::vec3(-1, 1, 0),glm::vec2(0, 1) }
	};
}

void Asset_Primitive::initialize(Engine * engine, const std::string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_primitive, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Primitive");
		initializeDefault(engine);
		return;
	}

	const size_t vertexCount = dataContainer.vertices.size();
	m_data.resize(vertexCount);
	for (size_t x = 0; x < vertexCount; ++x) {
		m_data[x].vertex = dataContainer.vertices[x];
		m_data[x].uv = dataContainer.texCoords[x];
	}
}

void Asset_Primitive::finalize(Engine * engine)
{
	// Load Buffers
	const size_t arraySize = m_data.size();
	glNamedBufferStorage(m_uboID, arraySize * sizeof(Single_Primitive_Vertex), &m_data[0], 0);		
	
	// Finalize
	Asset::finalize(engine);
}

size_t Asset_Primitive::getSize()
{
	return m_data.size();
}
