#include "Assets\Asset_Model.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"
#include <algorithm>

#define DIRECTORY_MODEL Engine::Get_Current_Dir() + "\\Models\\"


/** Calculates a Axis Aligned Bounding Box from a set of vertices.
Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
@param	vertices	the vertices of the mesh to derive the AABB from
@param	minOut	output reference containing the minimum extents of the AABB
@param	maxOut	output reference containing the maximum extents of the AABB */
inline void calculate_AABB(const std::vector<SingleVertex> & mesh, glm::vec3 & minOut, glm::vec3 & maxOut, glm::vec3 & centerOut, float & radiusOut)
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

/** Initialize a model's material, where each texture is specified individually.
@param	engine			the engine being used
@param	filename		the model's filename to use as a guide
@param	modelMaterial	the material asset to load into
@param	sceneMaterial	the scene material to use as a guide */
inline void generate_material(Engine * engine, const std::string & fullDirectory, Shared_Asset_Material & modelMaterial, const std::vector<Material> & materials)
{
	// Get texture names
	const size_t slash1Index = fullDirectory.find_last_of('/'), slash2Index = fullDirectory.find_last_of('\\');
	const size_t furthestFolderIndex = std::max(slash1Index != std::string::npos ? slash1Index : 0, slash2Index != std::string::npos ? slash2Index : 0);
	const std::string modelDirectory = fullDirectory.substr(0, furthestFolderIndex+1);
	std::vector<std::string> textures(materials.size() * (size_t)MAX_PHYSICAL_IMAGES);	
	for (size_t tx = 0, mx = 0; tx < textures.size() && mx < materials.size(); tx += MAX_PHYSICAL_IMAGES, ++mx) {
		textures[tx + 0] = modelDirectory + materials[mx].albedo;
		textures[tx + 1] = modelDirectory + materials[mx].normal;
		textures[tx + 2] = modelDirectory + materials[mx].metalness;
		textures[tx + 3] = modelDirectory + materials[mx].roughness;
		textures[tx + 4] = modelDirectory + materials[mx].height;
		textures[tx + 5] = modelDirectory + materials[mx].ao;
	}

	modelMaterial = Asset_Material::Create(engine, textures);
}

/** Initialize a model's materials, using the model's name as a lookup to an external material file.
@param	engine			the engine being used
@param	modelMaterial	the material asset to load into
@param	filename		the model's filename to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const std::string & filename)
{
	std::string materialFilename = filename.substr(filename.find("Models\\"));
	materialFilename = materialFilename.substr(0, materialFilename.find_first_of("."));
	modelMaterial = Asset_Material::Create(engine, materialFilename);
}

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

void Asset_Model::initializeDefault(Engine * engine)
{
	m_materialArray = Asset_Material::Create(engine, "defaultMaterial");
	
	// Create hard-coded alternative
	m_data.m_vertices = { 
		{glm::vec3(-1, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(1, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0) },
		{glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(-1, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)},
		{glm::vec3(-1, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0), m_materialArray->m_matSpot, glm::ivec4(0), glm::vec4(0)} 
	};
	
	calculate_AABB(m_data.m_vertices, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);
}

void Asset_Model::initialize(Engine * engine, const std::string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_model, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Model");
		initializeDefault(engine);
		return;
	}

	std::unique_lock<std::shared_mutex> m_asset_guard(m_mutex);
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
	

	calculate_AABB(m_data.m_vertices, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);

	// Generate all the required skins
	if (dataContainer.materials.size())
		generate_material(engine, fullDirectory, m_materialArray, dataContainer.materials);
	else
		generate_material(engine, m_materialArray, fullDirectory);

	for (size_t x = 0; x < vertexCount; ++x)
		m_data.m_vertices[x].matID = m_materialArray->m_matSpot;
}

void Asset_Model::finalize(Engine * engine)
{
	// Register geometry
	{
		std::shared_lock<std::shared_mutex> read_guard(m_mutex);
		m_modelManager->registerGeometry(m_data, m_offset, m_count);
	}
	Asset::finalize(engine);
}