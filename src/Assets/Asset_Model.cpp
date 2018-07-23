#include "Assets\Asset_Model.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"
#include <minmax.h>
#define EXT_MODEL ".obj"
#define DIRECTORY_MODEL Engine::Get_Current_Dir() + "\\Models\\"
#define ABS_DIRECTORY_MODEL(filename) DIRECTORY_MODEL + filename + EXT_MODEL
#define DIRECTORY_MODEL_MAT_TEX Engine::Get_Current_Dir() + "\\Textures\\Environment\\" 


/** Calculates a Axis Aligned Bounding Box from a set of vertices.\n
* Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
* @param	vertices	the vertices of the mesh to derive the AABB from
* @param	minOut	output reference containing the minimum extents of the AABB
* @param	maxOut	output reference containing the maximum extents of the AABB */
inline void calculate_AABB(const std::vector<glm::vec3> & vertices, glm::vec3 & minOut, glm::vec3 & maxOut, glm::vec3 & centerOut, float & radiusOut)
{
	if (vertices.size() >= 1) {
		const glm::vec3 & vector = vertices.at(0);
		float minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (int x = 1, total = vertices.size(); x < total; ++x) {
			const glm::vec3 &vertex = vertices.at(x);
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
* @param	engine			the engine being used
* @param	modelMaterial	the material asset to load into
* @param	sceneMaterial	the scene material to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const Material & material)
{
	// Get texture names
	std::string material_textures[6] = {
		DIRECTORY_MODEL_MAT_TEX + material.albedo,
		DIRECTORY_MODEL_MAT_TEX + material.normal,
		DIRECTORY_MODEL_MAT_TEX + material.metalness,
		DIRECTORY_MODEL_MAT_TEX + material.roughness,
		DIRECTORY_MODEL_MAT_TEX + material.height,
		DIRECTORY_MODEL_MAT_TEX + material.ao
	};

	engine->createAsset(modelMaterial, std::string(""), true, material_textures);
}

/** Initialize a model's materials, using the model's name as a lookup to an external material file.
* @param	engine			the engine being used
* @param	modelMaterial	the material asset to load into
* @param	filename		the model's filename to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const std::string & filename)
{
	std::string materialFilename = filename.substr(filename.find("Models\\"));
	materialFilename = materialFilename.substr(0, materialFilename.find_first_of("."));
	engine->createAsset(modelMaterial, materialFilename, true);
}

Asset_Model::~Asset_Model()
{
	if (existsYet())
		m_modelManager->unregisterGeometry(m_data, m_offset, m_count);
}

Asset_Model::Asset_Model(const std::string & filename, ModelManager * modelManager) : Asset(filename)
{
	m_meshSize = 0;
	m_bboxMin = glm::vec3(0.0f);
	m_bboxMax = glm::vec3(0.0f);
	m_bboxCenter = glm::vec3(0.0f);
	m_radius = 0.0f;
	m_offset = 0;
	m_count = 0;
	m_modelManager = modelManager;
}

void Asset_Model::CreateDefault(Engine * engine, Shared_Asset_Model & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	ModelManager & modelManager = engine->getModelManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultModel"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultModel", &modelManager);
	userAsset->m_data.vs = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	userAsset->m_data.uv = std::vector<glm::vec2>{ glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(0, 1) };
	userAsset->m_data.nm = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	userAsset->m_data.tg = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	userAsset->m_data.bt = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	userAsset->m_meshSize = 6; // Final vertex size (needed for draw arrays call)
	userAsset->m_data.bones.resize(6);
	userAsset->m_skins.resize(1);
	calculate_AABB(userAsset->m_data.vs, userAsset->m_bboxMin, userAsset->m_bboxMax, userAsset->m_bboxCenter, userAsset->m_radius);
	engine->createAsset(userAsset->m_skins[0], std::string("defaultMaterial"), true);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Model::Create(Engine * engine, Shared_Asset_Model & userAsset, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	ModelManager & modelManager = engine->getModelManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_MODEL + filename;
	if (!Engine::File_Exists(fullDirectory)) {
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
		CreateDefault(engine, userAsset);
		return;
	}

	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[engine, &userAsset, fullDirectory]() mutable { Initialize(engine, userAsset, fullDirectory); },
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
		/* Constructor Arguments. */
		filename, &modelManager
	);
}

void Asset_Model::Initialize(Engine * engine, Shared_Asset_Model & userAsset, const std::string & fullDirectory)
{
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_model, dataContainer)) {
		engine->reportError(MessageManager::OTHER_ERROR, "Failed to load model asset, using default...");
		CreateDefault(engine, userAsset);
		return;
	}

	std::unique_lock<std::shared_mutex> m_asset_guard(userAsset->m_mutex);
	userAsset->m_meshSize = dataContainer.vertices.size();
	userAsset->m_data.vs = dataContainer.vertices;
	userAsset->m_data.nm = dataContainer.normals;
	userAsset->m_data.tg = dataContainer.tangents;
	userAsset->m_data.bt = dataContainer.bitangents;
	userAsset->m_data.uv = dataContainer.texCoords;
	userAsset->m_data.bones = dataContainer.bones;
	userAsset->m_boneTransforms = dataContainer.boneTransforms;
	userAsset->m_boneMap = dataContainer.boneMap;
	userAsset->m_animations = dataContainer.animations;
	userAsset->m_rootNode = dataContainer.rootNode;

	calculate_AABB(userAsset->m_data.vs, userAsset->m_bboxMin, userAsset->m_bboxMax, userAsset->m_bboxCenter, userAsset->m_radius);

	// Generate all the required skins
	userAsset->m_skins.resize(max(1, (dataContainer.materials.size())));
	if (dataContainer.materials.size())
		for (int x = 0; x < dataContainer.materials.size(); ++x)
			generate_material(engine, userAsset->m_skins[x], dataContainer.materials[x]);
	else
		generate_material(engine, userAsset->m_skins[0], fullDirectory);
}

void Asset_Model::Finalize(Engine * engine, Shared_Asset_Model & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();

	// Register geometry
	std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
	userAsset->m_modelManager->registerGeometry(userAsset->m_data, userAsset->m_offset, userAsset->m_count);

	// Notify Completion
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.first, qwe.second);
}

GLuint Asset_Model::getSkinID(const unsigned int & desired)
{
	std::shared_lock<std::shared_mutex> guard(m_mutex);
	return m_skins[max(0, min(m_skins.size() - 1, desired))]->m_matSpot;
}