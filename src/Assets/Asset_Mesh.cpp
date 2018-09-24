#include "Assets\Asset_Mesh.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


Asset_Mesh::Asset_Mesh(const std::string & filename) : Asset(filename) {}

Shared_Asset_Mesh Asset_Mesh::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Mesh>(filename, threaded);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Mesh>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, filename);
		if (!Engine::File_Exists(filename)) {
			engine->reportError(MessageManager::FILE_MISSING, filename);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(std::move(initFunc), threaded);
	}
	return userAsset;
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
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Mesh");
		initializeDefault(engine);
	}

	Asset::finalize(engine);
}