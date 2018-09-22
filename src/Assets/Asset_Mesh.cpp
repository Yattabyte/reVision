#include "Assets\Asset_Mesh.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"
#include <chrono>
#include <thread>

#define DIRECTORY_MESH Engine::Get_Current_Dir()


Asset_Mesh::~Asset_Mesh()
{
}

Asset_Mesh::Asset_Mesh(const std::string & filename) : Asset(filename) {}

Shared_Asset_Mesh Asset_Mesh::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Mesh>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Mesh>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_MESH + filename;
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory)) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	else {
		if (!threaded) {
			// If we need the asset right away and it is already found
			// It is possible that it was initially created while threaded
			// Hence, we must wait for it to be completed here
			while (!userAsset->existsYet()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
		}
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

void Asset_Mesh::initialize(Engine * engine, const std::string & fullDirectory)
{
	if (!Mesh_IO::Import_Model(engine, fullDirectory, m_geometry)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Mesh");
		initializeDefault(engine);
		return;
	}
}

void Asset_Mesh::finalize(Engine * engine)
{
	// Finalize
	Asset::finalize(engine);
}