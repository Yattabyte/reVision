#include "Assets\Asset_Model.h"
#include "Engine.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include <minmax.h>


/** Calculates a Axis Aligned Bounding Box from a set of vertices.\n
 * Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
 * @param	vertices	the vertices of the mesh to derive the AABB from
 * @param	minOut	output reference containing the minimum extents of the AABB
 * @param	maxOut	output reference containing the maximum extents of the AABB */
inline void calculate_AABB(const vector<vec3> & vertices, vec3 & minOut, vec3 & maxOut, vec3 & centerOut, float & radiusOut)
{
	if (vertices.size() >= 1) {
		const vec3 &vector = vertices.at(0);
		float minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (int x = 1, total = vertices.size(); x < total; ++x) {
			const vec3 &vertex = vertices.at(x);
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

		minOut = vec3(minX, minY, minZ);
		maxOut = vec3(maxX, maxY, maxZ);
		centerOut = ((maxOut - minOut) / 2.0f) + minOut;
		radiusOut = glm::distance(minOut, maxOut) / 2.0f;
	}
}

/** Convert an aiMatrix to glm::mat4x4.
 * @param	d	the aiMatrix to convert from
 * @return		the glm::mat4x4 converted to */
inline mat4 aiMatrix_to_Mat4x4(const aiMatrix4x4 &d)
{
	return mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

/** Initialize a model's bones.
 * @param	model	the model to use
 * @param	scene	the scene to use */
inline void initialize_bones(Shared_Asset_Model & model, const aiScene * scene)
{
	vector<BoneInfo> &boneInfo = model->m_animationInfo.meshTransforms;
	map<string, int> &m_BoneMapping = model->m_animationInfo.boneMap;
	vector<VertexBoneData> &bones = model->m_data.bones;

	int vertexOffset = 0;

	for (int A = 0, meshTotal = scene->mNumMeshes; A < meshTotal; A++) {

		const aiMesh *mesh = scene->mMeshes[A];

		for (int B = 0, numBones = mesh->mNumBones; B < numBones; B++) {
			int BoneIndex = 0;
			string BoneName(mesh->mBones[B]->mName.data);

			if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
				BoneIndex = boneInfo.size();
				BoneInfo bi;
				boneInfo.push_back(bi);
			}
			else
				BoneIndex = m_BoneMapping[BoneName];

			m_BoneMapping[BoneName] = BoneIndex;
			boneInfo[BoneIndex].BoneOffset = aiMatrix_to_Mat4x4(mesh->mBones[B]->mOffsetMatrix);

			for (unsigned int j = 0; j < mesh->mBones[B]->mNumWeights; j++) {
				int VertexID = vertexOffset + mesh->mBones[B]->mWeights[j].mVertexId;
				float Weight = mesh->mBones[B]->mWeights[j].mWeight;
				bones[VertexID].AddBoneData(BoneIndex, Weight);
			}
		}

		for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const aiFace& face = mesh->mFaces[x];
			for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b)
				vertexOffset++;
		}
	}
}

/** Initialize a model's material, where each texture is specified individually. 
* @param	engine			the engine being used
 * @param	modelMaterial	the material asset to load into
 * @param	sceneMaterial	the scene material to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const aiMaterial * sceneMaterial)
{
	// Get the aiStrings for all the textures for a material
	aiString	albedo, normal, metalness, roughness, height, ao;
	aiReturn	albedo_exists = sceneMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &albedo),
				normal_exists = sceneMaterial->GetTexture(aiTextureType_NORMALS, 0, &normal),
				metalness_exists = sceneMaterial->GetTexture(aiTextureType_SPECULAR, 0, &metalness),
				roughness_exists = sceneMaterial->GetTexture(aiTextureType_SHININESS, 0, &roughness),
				height_exists = sceneMaterial->GetTexture(aiTextureType_HEIGHT, 0, &height),
				ao_exists = sceneMaterial->GetTexture(aiTextureType_AMBIENT, 0, &ao);

	// Assuming the diffuse element exists, generate some fallback texture elements
	std::string templateTexture, extension = ".png";
	if (albedo_exists == AI_SUCCESS) {
		std::string minusD = albedo.C_Str();
		int exspot = minusD.find_last_of(".");
		extension = minusD.substr(exspot, minusD.length());
		int diffuseStart = minusD.find("diff");
		if (diffuseStart > -1)
			minusD = minusD.substr(0, diffuseStart);
		else
			minusD = minusD.substr(0, exspot) + "_";
		templateTexture = minusD;
	}

	// Importer might not distinguish between height and normal maps
	if (normal_exists != AI_SUCCESS && height_exists == AI_SUCCESS) {
		std::string norm_string(height.C_Str());
		const int norm_spot = norm_string.find_last_of("norm");
		if (norm_spot > -1) {
			// Normal map confirmed to be in height map spot, move it over
			normal = height;
			normal_exists = AI_SUCCESS;
			height_exists = AI_FAILURE;
		}
	}

	// Get texture names
	std::string material_textures[6] = {
		/*ALBEDO*/						DIRECTORY_MODEL_MAT_TEX + (albedo_exists == AI_SUCCESS ? albedo.C_Str() : "albedo.png"),
		/*NORMAL*/						DIRECTORY_MODEL_MAT_TEX + (normal_exists == AI_SUCCESS ? normal.C_Str() : templateTexture + "normal" + extension),
		/*METALNESS*/					DIRECTORY_MODEL_MAT_TEX + (metalness_exists == AI_SUCCESS ? metalness.C_Str() : templateTexture + "metalness" + extension),
		/*ROUGHNESS*/					DIRECTORY_MODEL_MAT_TEX + (roughness_exists == AI_SUCCESS ? roughness.C_Str() : templateTexture + "roughness" + extension),
		/*HEIGHT*/						DIRECTORY_MODEL_MAT_TEX + (height_exists == AI_SUCCESS ? height.C_Str() : templateTexture + "height" + extension),
		/*AO*/							DIRECTORY_MODEL_MAT_TEX + (ao_exists == AI_SUCCESS ? ao.C_Str() : templateTexture + "ao" + extension)
	};

	engine->createAsset(modelMaterial, string(""), true, material_textures);
}

/** Initialize a model's materials, using the model's name as a lookup to an external material file.
* @param	engine			the engine being used
 * @param	modelMaterial	the material asset to load into
 * @param	filename		the model's filename to use as a guide */
inline void generate_material(Engine * engine, Shared_Asset_Material & modelMaterial, const string & filename)
{
	std::string materialFilename = filename.substr(filename.find("Models\\"));
	materialFilename = materialFilename.substr(0, materialFilename.find_first_of("."));
	engine->createAsset(modelMaterial, materialFilename, true);
}

// Scene gets destroyed at the end of asset creation
// We need to copy animation related information
void AnimationInfo::setScene(const aiScene * scene) 
{
	Animations.resize(scene->mNumAnimations);
	for (int x = 0, total = scene->mNumAnimations; x < total; ++x)
		Animations[x] = new aiAnimation(*scene->mAnimations[x]);

	RootNode = new aiNode(*scene->mRootNode);
}

size_t AnimationInfo::numAnimations() const 
{
	return Animations.size(); 
}

Asset_Model::~Asset_Model()
{
	if (existsYet())
		m_modelManager->unregisterGeometry(m_data, m_offset, m_count);
}

Asset_Model::Asset_Model(const string & filename, ModelManager * modelManager) : Asset(filename)
{
	m_meshSize = 0;
	m_bboxMin = vec3(0.0f);
	m_bboxMax = vec3(0.0f);
	m_bboxCenter = vec3(0.0f);
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
	userAsset->m_data.vs = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_data.uv = vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
	userAsset->m_data.nm = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_data.tg = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_data.bt = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	userAsset->m_meshSize = 6; // Final vertex size (needed for draw arrays call)
	userAsset->m_data.bones.resize(6);
	userAsset->m_skins.resize(1);
	calculate_AABB(userAsset->m_data.vs, userAsset->m_bboxMin, userAsset->m_bboxMax, userAsset->m_bboxCenter, userAsset->m_radius);
	engine->createAsset(userAsset->m_skins[0], string("defaultMaterial"), true);

	// Create the asset
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Model::Create(Engine * engine, Shared_Asset_Model & userAsset, const string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();
	ModelManager & modelManager = engine->getModelManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = DIRECTORY_MODEL + filename;
	if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
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

void Asset_Model::Initialize(Engine * engine, Shared_Asset_Model & userAsset, const string & fullDirectory)
{
	Assimp::Importer &importer = *new Assimp::Importer();
	const aiScene* scene = importer.ReadFile(fullDirectory,
		aiProcess_LimitBoneWeights |
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace /*|
								   aiProcess_SplitLargeMeshes |
								   aiProcess_OptimizeMeshes |
								   aiProcess_OptimizeGraph |
								   aiProcess_GenSmoothNormals
								   aiProcess_ImproveCacheLocality*/);
								   // Scene cannot be read
	if (!scene) {
		engine->reportError(MessageManager::FILE_CORRUPT, fullDirectory);
		CreateDefault(engine, userAsset);
		return;
	}

	unique_lock<shared_mutex> m_asset_guard(userAsset->m_mutex);
	GeometryInfo &gi = userAsset->m_data;

	// Combine mesh data into single struct
	for (int a = 0, atotal = scene->mNumMeshes; a < atotal; ++a) {
		const aiMesh *mesh = scene->mMeshes[a];
		for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const aiFace& face = mesh->mFaces[x];
			for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b) {
				const int index = face.mIndices[b];
				const aiVector3D vertex = mesh->mVertices[index];
				const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[index] : aiVector3D(1.0f, 1.0f, 1.0f);
				const aiVector3D tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[index] : aiVector3D(1.0f, 1.0f, 1.0f);
				const aiVector3D bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[index] : aiVector3D(1.0f, 1.0f, 1.0f);
				const aiVector3D uvmap = mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][index]) : aiVector3D(0, 0, 0);
				gi.vs.push_back(vec3(vertex.x, vertex.y, vertex.z));
				gi.nm.push_back(glm::normalize(vec3(normal.x, normal.y, normal.z)));
				gi.tg.push_back(glm::normalize(vec3(tangent.x, tangent.y, tangent.z) - vec3(normal.x, normal.y, normal.z) * glm::dot(vec3(normal.x, normal.y, normal.z), vec3(tangent.x, tangent.y, tangent.z))));
				gi.bt.push_back(vec3(bitangent.x, bitangent.y, bitangent.z));
				gi.uv.push_back(vec2(uvmap.x, uvmap.y));
			}
		}
	}
	userAsset->m_animationInfo.setScene(scene);
	userAsset->m_meshSize = gi.vs.size(); // Final vertex size (needed for draw arrays call)
	gi.bones.resize(gi.vs.size());
	calculate_AABB(gi.vs, userAsset->m_bboxMin, userAsset->m_bboxMax, userAsset->m_bboxCenter, userAsset->m_radius);
	initialize_bones(userAsset, scene);

	// Generate all the required skins
	userAsset->m_skins.resize(max(1, (scene->mNumMaterials - 1)));
	if (scene->mNumMaterials > 1)
		for (int x = 1; x < scene->mNumMaterials; ++x) // ignore scene material [0] 
			generate_material(engine, userAsset->m_skins[x - 1], scene->mMaterials[x]);
	else
		generate_material(engine, userAsset->m_skins[0], fullDirectory);
}

void Asset_Model::Finalize(Engine * engine, Shared_Asset_Model & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_finalized = true;
	userAsset->m_modelManager->registerGeometry(userAsset->m_data, userAsset->m_offset, userAsset->m_count);
	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
	/* To Do: Finalize call here*/
}

GLuint Asset_Model::getSkinID(const unsigned int & desired)
{
	shared_lock<shared_mutex> guard(m_mutex);
	return m_skins[max(0, min(m_skins.size() - 1, desired))]->m_matSpot;
}