#include "Assets\Asset_Model.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"
#include <algorithm>

#define DIRECTORY_MODEL Engine::Get_Current_Dir() + "\\Models\\"


Asset_Model::~Asset_Model()
{
	if (existsYet())
		m_modelManager->unregisterGeometry(m_data, m_offset, m_count);
}

Asset_Model::Asset_Model(const std::string & filename, ModelManager & modelManager) : Asset(filename), m_modelManager(&modelManager) {}

Shared_Asset_Model Asset_Model::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	ModelManager & modelManager = engine->getModelManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Model>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Model>(filename, modelManager);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_MODEL + filename;
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

void Asset_Model::calculateAABB(const std::vector<SingleVertex>& mesh, glm::vec3 & minOut, glm::vec3 & maxOut, glm::vec3 & centerOut, float & radiusOut)
{
	if (mesh.size() >= 1) {
		const glm::vec3 & vector = mesh[0].vertex;
		float minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (size_t x = 1, total = mesh.size(); x < total; ++x) {
			const glm::vec3 &vertex = mesh[x].vertex;
			if (vertex.x < minX)
				minX = vertex.x;
			else if (vertex.x > maxX)
				maxX = vertex.x;
			if (vertex.y < minY)
				minY = vertex.y;
			else if (vertex.y > maxY)
				maxY = vertex.y;
			if (vertex.z < minZ)
				minZ = vertex.z;
			else if (vertex.z > maxZ)
				maxZ = vertex.z;
		}

		minOut = glm::vec3(minX, minY, minZ);
		maxOut = glm::vec3(maxX, maxY, maxZ);
		centerOut = ((maxOut - minOut) / 2.0f) + minOut;
		radiusOut = glm::distance(minOut, maxOut) / 2.0f;
	}
}

void Asset_Model::loadMaterial(Engine * engine, const std::string & fullDirectory, Shared_Asset_Material & modelMaterial, const std::vector<Material>& materials)
{
	// Retrieve texture directories from the model file
	const size_t slash1Index = fullDirectory.find_last_of('/'), slash2Index = fullDirectory.find_last_of('\\');
	const size_t furthestFolderIndex = std::max(slash1Index != std::string::npos ? slash1Index : 0, slash2Index != std::string::npos ? slash2Index : 0);
	const std::string modelDirectory = fullDirectory.substr(0, furthestFolderIndex + 1);
	std::vector<std::string> textures(materials.size() * (size_t)MAX_PHYSICAL_IMAGES);
	for (size_t tx = 0, mx = 0; tx < textures.size() && mx < materials.size(); tx += MAX_PHYSICAL_IMAGES, ++mx) {
		textures[tx + 0] = modelDirectory + materials[mx].albedo;
		textures[tx + 1] = modelDirectory + materials[mx].normal;
		textures[tx + 2] = modelDirectory + materials[mx].metalness;
		textures[tx + 3] = modelDirectory + materials[mx].roughness;
		textures[tx + 4] = modelDirectory + materials[mx].height;
		textures[tx + 5] = modelDirectory + materials[mx].ao;
	}

	// Attempt to find a .mat file if it exists
	std::string materialFilename = fullDirectory.substr(0, fullDirectory.find_first_of(".")) + ".mat";
	modelMaterial = Asset_Material::Create(engine, materialFilename, textures);
}

void Asset_Model::initializeDefault(Engine * engine)
{
	m_materialArray = Asset_Material::Create(engine, "defaultMaterial", {"albedo.png","normal.png","metalness.png","roughness.png","height.png","occlusion.png"});
	
	// Create hard-coded alternative
	m_data.m_vertices = { 
		{glm::vec3(-1, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(1, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0) },
		{glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(-1, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(-1, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)} 
	};
	
	calculateAABB(m_data.m_vertices, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);
}

void Asset_Model::initialize(Engine * engine, const std::string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_model, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Model");
		initializeDefault(engine);
		return;
	}

	const size_t vertexCount = dataContainer.vertices.size();
	m_data.m_vertices.resize(vertexCount);
	for (size_t x = 0; x < vertexCount; ++x) {
		m_data.m_vertices[x].vertex = dataContainer.vertices[x];
		m_data.m_vertices[x].normal = dataContainer.normals[x];
		m_data.m_vertices[x].tangent = dataContainer.tangents[x];
		m_data.m_vertices[x].bitangent = dataContainer.bitangents[x];
		m_data.m_vertices[x].uv = dataContainer.texCoords[x];
		m_data.m_vertices[x].boneIDs.x = dataContainer.bones[x].IDs[0];
		m_data.m_vertices[x].boneIDs.y = dataContainer.bones[x].IDs[1];
		m_data.m_vertices[x].boneIDs.z = dataContainer.bones[x].IDs[2];
		m_data.m_vertices[x].boneIDs.w = dataContainer.bones[x].IDs[3];
		m_data.m_vertices[x].weights.x = dataContainer.bones[x].Weights[0];
		m_data.m_vertices[x].weights.y = dataContainer.bones[x].Weights[1];
		m_data.m_vertices[x].weights.z = dataContainer.bones[x].Weights[2];
		m_data.m_vertices[x].weights.w = dataContainer.bones[x].Weights[3];
	}
	m_boneTransforms = dataContainer.boneTransforms;
	m_boneMap = dataContainer.boneMap;
	m_animations = dataContainer.animations;
	m_rootNode = dataContainer.rootNode;
	
	// Calculate the model's min, max, center, and radius
	calculateAABB(m_data.m_vertices, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);

	// Generate all the required skins
	loadMaterial(engine, fullDirectory, m_materialArray, dataContainer.materials);

	// Apply this model's material set index to each vertex
	for (size_t x = 0; x < vertexCount; ++x)
		m_data.m_vertices[x].matID = m_materialArray->m_matSpot;
}

void Asset_Model::finalize(Engine * engine)
{
	// Register geometry
	m_modelManager->registerGeometry(m_data, m_offset, m_count);
	
	// Finalize
	Asset::finalize(engine);
}