#include "Assets/Mesh.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"


Shared_Mesh::Shared_Mesh(Engine * engine, const std::string & filename, const bool & threaded)
{
	(*(std::shared_ptr<Mesh>*)(this)) = std::dynamic_pointer_cast<Mesh>(
		engine->getManager_Assets().shareAsset(
			typeid(Mesh).name(),
			filename,
			[engine, filename]() { return std::make_shared<Mesh>(engine, filename); },
			threaded
		));
}

Mesh::Mesh(Engine * engine, const std::string & filename) : Asset(engine, filename) {}

void Mesh::initializeDefault()
{
	// Create hard-coded alternative
	m_geometry.vertices = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0),glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	m_geometry.normals = { glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0) };
	m_geometry.tangents = { glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) };
	m_geometry.bitangents = { glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1) };
	m_geometry.texCoords = { glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0) ,glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0) };
}

void Mesh::initialize()
{
	if (!Mesh_IO::Import_Model(m_engine, getFileName(), m_geometry)) {
		m_engine->getManager_Messages().error("Mesh \"" + m_filename + "\" failed to initialize.");
		initializeDefault();
	}

	Asset::finalize();
}