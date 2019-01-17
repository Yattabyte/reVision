#include "Assets/Asset_Mesh.h"
#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"


Shared_Mesh::Shared_Mesh(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Mesh>(std::dynamic_pointer_cast<Asset_Mesh>(engine->getManager_Assets().shareAsset(typeid(Asset_Mesh).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Asset_Mesh>*)(this)) = std::make_shared<Asset_Mesh>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Asset_Mesh).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Asset_Mesh::initialize, get(), engine, filename)), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Asset_Mesh::Asset_Mesh(const std::string & filename) : Asset(filename) {}

void Asset_Mesh::initializeDefault()
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
		engine->getManager_Messages().error("Asset_Mesh \"" + m_filename + "\" failed to initialize.");
		initializeDefault();
	}

	Asset::finalize(engine);
}