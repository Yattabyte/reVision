#include "Assets\Asset_Mesh.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


Asset_Mesh::Asset_Mesh(const std::string & filename) : Asset(filename) {}

Shared_Asset_Mesh Asset_Mesh::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Mesh>(
		filename,
		"",
		"",
		&initialize,
		engine,
		threaded
	);
}

void Asset_Mesh::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_geometry.vertices = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0),glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	m_geometry.normals = { glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0),  glm::vec3(1, 0, 0) };
	m_geometry.tangents = { glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) };
	m_geometry.bitangents = { glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1) };
	m_geometry.texCoords = { glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0) ,glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(0, 0) };
}

void Asset_Mesh::initialize(Engine * engine, const std::string & relativePath)
{
	if (!Mesh_IO::Import_Model(engine, relativePath, m_geometry)) {
		engine->getMessageManager().error(MessageManager::ASSET_FAILED, "Asset_Mesh");
		initializeDefault(engine);
	}

	Asset::finalize(engine);
}