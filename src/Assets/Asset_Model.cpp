#include "Assets\Asset_Model.h"
#include "Managers\Message_Manager.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include <minmax.h>


AnimationInfo::~AnimationInfo() 
{
}

AnimationInfo::AnimationInfo()
{
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
	if (m_fence != nullptr)
		glDeleteSync(m_fence);

	if (existsYet())
		Asset_Manager::Get_Model_Manager()->unregisterGeometry(m_data, m_offset, m_count);
}

Asset_Model::Asset_Model(const string & filename) : Asset(filename)
{
	m_meshSize = 0;
	m_bboxMin = vec3(0.0f);
	m_bboxMax = vec3(0.0f);
	m_bboxCenter = vec3(0.0f);
	m_radius = 0.0f;
	m_offset = 0;
	m_count = 0;
	m_fence = nullptr;
}

bool Asset_Model::existsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::existsYet() && m_fence != nullptr) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED))
			return true;
	}
	return false;
}

GLuint Asset_Model::getSkinID(const unsigned int & desired)
{
	shared_lock<shared_mutex> guard(m_mutex);
	return m_skins[max(0, min(m_skins.size() - 1, desired))]->m_matSpot;
}

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Model & asset)
{	
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Model>(asset, "defaultModel"))
		return;

	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Model>(asset, "defaultModel");
	asset->m_data.vs = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->m_data.uv= vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
	asset->m_data.nm = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->m_data.tg = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->m_data.bt = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->m_bboxMin = vec3(-1);
	asset->m_bboxMax = vec3(1);
	asset->m_data.bones.resize(6);
	asset->m_skins.resize(1);
	Asset_Loader::load_asset(asset->m_skins[0], "defaultMaterial");
	Asset_Manager::Add_Work_Order(new Model_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Model & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::Query_Existing_Asset<Asset_Model>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_MODEL + filename;
		if (!File_Reader::FileExistsOnDisk(fullDirectory)) {
			MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
			fetch_default_asset(user);
			return;
		}

		// Create the asset
		Asset_Manager::Submit_New_Asset<Asset_Model, Model_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

/** Calculates a Axis Aligned Bounding Box from a set of vertices.\n
 * Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
 * @param	vertices	the vertices of the mesh to derive the AABB from
 * @param	minOut	output reference containing the minimum extents of the AABB
 * @param	maxOut	output reference containing the maximum extents of the AABB */
void calculate_AABB(const vector<vec3> & vertices, vec3 & minOut, vec3 & maxOut, vec3 & centerOut, float & radiusOut)
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

void Model_WorkOrder::initializeOrder()
{	
	Assimp::Importer &importer = *new Assimp::Importer();
	const aiScene* scene = importer.ReadFile(m_filename,
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
		MSG_Manager::Error(MSG_Manager::FILE_CORRUPT, m_filename);
		fetch_default_asset(m_asset);
		return;
	}

	unique_lock<shared_mutex> m_asset_guard(m_asset->m_mutex);
	GeometryInfo &gi = m_asset->m_data;

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
	m_asset->m_animationInfo.setScene(scene);
	m_asset->m_meshSize = gi.vs.size(); // Final vertex size (needed for draw arrays call)
	gi.bones.resize(gi.vs.size());
	calculate_AABB(gi.vs, m_asset->m_bboxMin, m_asset->m_bboxMax, m_asset->m_bboxCenter, m_asset->m_radius);
	initializeBones(m_asset, scene);

	// Generate all the required skins
	m_asset->m_skins.resize(max(1, (scene->mNumMaterials - 1)));
	if (scene->mNumMaterials > 1) 		
		for (int x = 1; x < scene->mNumMaterials; ++x) // ignore scene material [0] 
			generateMaterial(m_asset->m_skins[x - 1], scene->mMaterials[x]);	
	else
		generateMaterial(m_asset->m_skins[0]);
}

void Model_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		Asset_Manager::Get_Model_Manager()->registerGeometry(m_asset->m_data, m_asset->m_offset, m_asset->m_count);
		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		write_guard.unlock();
		write_guard.release();
		m_asset->finalize();
	}
}
mat4 inline aiMatrixtoMat4x4(const aiMatrix4x4 &d)
{
	return mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

void Model_WorkOrder::initializeBones(Shared_Asset_Model & model, const aiScene * scene)
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
			boneInfo[BoneIndex].BoneOffset = aiMatrixtoMat4x4(mesh->mBones[B]->mOffsetMatrix);

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

void Model_WorkOrder::generateMaterial(Shared_Asset_Material & modelMaterial, const aiMaterial * sceneMaterial)
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

	Asset_Loader::load_asset(modelMaterial, material_textures);
}

void Model_WorkOrder::generateMaterial(Shared_Asset_Material & modelMaterial)
{
	std::string materialFilename = m_filename.substr(m_filename.find("Models\\"));
	materialFilename = materialFilename.substr(0, materialFilename.find_first_of("."));
	Asset_Loader::load_asset(modelMaterial, materialFilename);
}
