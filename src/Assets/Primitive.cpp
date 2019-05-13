#include "Assets/Primitive.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"


constexpr char* EXT_PRIMITIVE = ".obj";
constexpr char* DIRECTORY_PRIMITIVE = "\\Primitives\\";

Shared_Primitive::Shared_Primitive(Engine * engine, const std::string & filename, const bool & threaded)
{
	(*(std::shared_ptr<Primitive>*)(this)) = std::dynamic_pointer_cast<Primitive>(
		engine->getManager_Assets().shareAsset(
			typeid(Primitive).name(),
			filename,
			[engine, filename]() { return std::make_shared<Primitive>(engine, filename); },
			threaded
		));
}

Primitive::~Primitive()
{
	if (existsYet())
		glDeleteBuffers(1, &m_uboID);
}

Primitive::Primitive(Engine * engine, const std::string & filename) : Asset(engine, filename) 
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

void Primitive::initialize()
{
	// Forward asset creation
	m_mesh = Shared_Mesh(m_engine, DIRECTORY_PRIMITIVE + getFileName() + EXT_PRIMITIVE, false);

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
	Asset::finalize();
}

size_t Primitive::getSize() const
{
	return m_data.size();
}