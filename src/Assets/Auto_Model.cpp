#include "Assets/Auto_Model.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"
#include "glm/glm.hpp"


constexpr const char* EXT_PRIMITIVE = ".obj";
constexpr const char* DIRECTORY_PRIMITIVE = "\\Models\\";

struct Single_Primitive_Vertex {
	glm::vec3 vertex;
	glm::vec2 uv;
	unsigned int meshID;
	glm::vec3 normal;
};

Shared_Auto_Model::Shared_Auto_Model(Engine& engine, const std::string& filename, const bool& threaded)
{
	auto newAsset = std::dynamic_pointer_cast<Auto_Model>(engine.getManager_Assets().shareAsset(
			typeid(Auto_Model).name(),
			filename,
			[&engine, filename]() { return std::make_shared<Auto_Model>(engine, filename); },
			threaded
		));
	swap(newAsset);
}

Auto_Model::~Auto_Model() noexcept
{
	if (ready())
		glDeleteBuffers(1, &m_vboID);
}

Auto_Model::Auto_Model(Engine& engine, const std::string& filename) : Asset(engine, filename)
{
	glCreateVertexArrays(1, &m_vaoID);
	glEnableVertexArrayAttrib(m_vaoID, 0);
	glEnableVertexArrayAttrib(m_vaoID, 1);
	glEnableVertexArrayAttrib(m_vaoID, 2);
	glEnableVertexArrayAttrib(m_vaoID, 3);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribBinding(m_vaoID, 1, 0);
	glVertexArrayAttribBinding(m_vaoID, 2, 0);
	glVertexArrayAttribBinding(m_vaoID, 3, 0);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Single_Primitive_Vertex, vertex));
	glVertexArrayAttribFormat(m_vaoID, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Single_Primitive_Vertex, uv));
	glVertexArrayAttribIFormat(m_vaoID, 2, 1, GL_UNSIGNED_INT, offsetof(Single_Primitive_Vertex, meshID));
	glVertexArrayAttribFormat(m_vaoID, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Single_Primitive_Vertex, normal));
	glCreateBuffers(1, &m_vboID);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(Single_Primitive_Vertex));
}

void Auto_Model::initialize()
{
	// Forward asset creation
	m_mesh = Shared_Mesh(m_engine, DIRECTORY_PRIMITIVE + getFileName() + EXT_PRIMITIVE, false);

	const size_t vertexCount = m_mesh->m_geometry.vertices.size();
	m_data.resize(vertexCount);
	for (size_t x = 0; x < vertexCount; ++x) {
		m_data[x].vertex = m_mesh->m_geometry.vertices[x];
		m_data[x].normal = m_mesh->m_geometry.normals[x];
		m_data[x].uv = m_mesh->m_geometry.texCoords[x];
		m_data[x].meshID = m_mesh->m_geometry.meshIndices[x];
	}

	// Load Buffers
	const size_t arraySize = m_data.size();
	glNamedBufferStorage(m_vboID, arraySize * sizeof(Single_Primitive_Vertex), &m_data[0], 0);

	// Finalize
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	Asset::finalize();
}

size_t Auto_Model::getSize() const noexcept
{
	return m_data.size();
}

void Auto_Model::bind() noexcept
{
	glBindVertexArray(m_vaoID);
}