#include "Assets\Asset_Model.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"

#define EXT_MODEL ".obj"
#define DIRECTORY_MODEL Engine::Get_Current_Dir() + "\\Models\\"
#define ABS_DIRECTORY_MODEL(filename) DIRECTORY_MODEL + filename + EXT_MODEL
#define DIRECTORY_MODEL_MAT_TEX Engine::Get_Current_Dir() + "\\Textures\\Environment\\" 


/** Calculates a Axis Aligned Bounding Box from a set of vertices.
Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
@param	vertices	the vertices of the mesh to derive the AABB from
@param	minOut	output reference containing the minimum extents of the AABB
@param	maxOut	output reference containing the maximum extents of the AABB */
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
@param	engine			the engine being used
@param	modelMaterial	the material asset to load into
@param	sceneMaterial	the scene material to use as a guide */
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

	modelMaterial = Asset_Material::Create(engine, material_textures);
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

Asset_Model::Asset_Model(const std::string & filename, ModelManager & modelManager) : Asset(filename)
{
	m_bboxMin = glm::vec3(0.0f);
	m_bboxMax = glm::vec3(0.0f);
	m_bboxCenter = glm::vec3(0.0f);
	m_radius = 0.0f;
	m_offset = 0;
	m_count = 0;
	m_modelManager = &modelManager;
}

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
	// Create hard-coded alternative
	m_data.vs = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	m_data.uv = std::vector<glm::vec2>{ glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(0, 1) };
	m_data.nm = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	m_data.tg = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };
	m_data.bt = std::vector<glm::vec3>{ glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0) };	
	m_data.bones.resize(6);
	m_skins.resize(1);
	calculate_AABB(m_data.vs, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);
	m_skins[0] = Asset_Material::Create(engine, "defaultMaterial");
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
	m_data.vs = dataContainer.vertices;
	m_data.nm = dataContainer.normals;
	m_data.tg = dataContainer.tangents;
	m_data.bt = dataContainer.bitangents;
	m_data.uv = dataContainer.texCoords;
	m_data.bones = dataContainer.bones;
	m_boneTransforms = dataContainer.boneTransforms;
	m_boneMap = dataContainer.boneMap;
	m_animations = dataContainer.animations;
	m_rootNode = dataContainer.rootNode;

	calculate_AABB(m_data.vs, m_bboxMin, m_bboxMax, m_bboxCenter, m_radius);

	// Generate all the required skins
	m_skins.resize(std::max(size_t(1), (dataContainer.materials.size())));
	if (dataContainer.materials.size())
		for (int x = 0; x < dataContainer.materials.size(); ++x)
			generate_material(engine, m_skins[x], dataContainer.materials[x]);
	else
		generate_material(engine, m_skins[0], fullDirectory);
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

GLuint Asset_Model::getSkinID(const unsigned int & desired)
{
	std::shared_lock<std::shared_mutex> guard(m_mutex);
	return m_skins[std::max(size_t(0), std::min(m_skins.size() - size_t(1), size_t(desired)))]->m_matSpot;
}