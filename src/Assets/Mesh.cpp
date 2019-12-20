#include "Assets/Mesh.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"
#include "glm/glm.hpp"
#include "glm/geometric.hpp"


Shared_Mesh::Shared_Mesh(Engine& engine, const std::string& filename, const bool& threaded)
{
	auto newAsset = std::dynamic_pointer_cast<Mesh>(engine.getManager_Assets().shareAsset(
			typeid(Mesh).name(),
			filename,
			[&engine, filename]() { return std::make_shared<Mesh>(engine, filename); },
			threaded
		));
	swap(newAsset);
}

Mesh::Mesh(Engine& engine, const std::string& filename) : Asset(engine, filename) {}

void Mesh::initialize()
{
	if (!Mesh_IO::Import_Model(m_engine, getFileName(), m_geometry)) {
		// Create hard-coded alternative
		m_geometry.vertices = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0),glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
		m_geometry.normals = { glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0) };
		m_geometry.tangents = { glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) };
		m_geometry.bitangents = { glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1) };
		m_geometry.texCoords = { glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0) };
		m_geometry.materialIndices = { 0U, 0U, 0U, 0U, 0U, 0U };
		m_geometry.bones.resize(m_geometry.vertices.size());
		m_geometry.materials.emplace_back("", "", "", "", "", "");
	}

	Asset::finalize();
}