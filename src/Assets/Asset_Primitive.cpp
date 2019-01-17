#include "Assets/Asset_Primitive.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"


constexpr char* EXT_PRIMITIVE = ".obj";
constexpr char* DIRECTORY_PRIMITIVE = "\\Primitives\\";

Shared_Primitive::Shared_Primitive(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Primitive>(std::dynamic_pointer_cast<Asset_Primitive>(engine->getManager_Assets().shareAsset(typeid(Asset_Primitive).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Asset_Primitive>*)(this)) = std::make_shared<Asset_Primitive>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Asset_Primitive).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Asset_Primitive::initialize, get(), engine, (DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

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

void Asset_Primitive::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_mesh = Shared_Mesh(engine, relativePath, false);

	const size_t vertexCount = m_mesh->m_geometry.vertices.size();
	m_data.resize(vertexCount);
	for (size_t x = 0; x < vertexCount; ++x) {
		m_data[x].vertex = m_mesh->m_geometry.vertices[x];
		m_data[x].uv = m_mesh->m_geometry.texCoords[x];
	}

	// Load Buffers
	const size_t arraySize = m_data.size();
	glNamedBufferStorage(m_uboID, arraySize * sizeof(Single_Primitive_Vertex), &m_data[0], 0);

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize(engine);
}

size_t Asset_Primitive::getSize() const
{
	return m_data.size();
}