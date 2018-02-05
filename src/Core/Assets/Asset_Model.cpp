#include "Assets\Asset_Model.h"
#include "Systems\Message_Manager.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include <minmax.h>

/* -----ASSET TYPE----- */
#define ASSET_TYPE 4

using namespace Asset_Loader;

VertexBoneData::~VertexBoneData()
{
}

VertexBoneData::VertexBoneData()
{
	Reset();
}

VertexBoneData::VertexBoneData(const VertexBoneData & vbd) 
{
	Reset();
	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
		IDs[i] = vbd.IDs[i];
		Weights[i] = vbd.Weights[i];
	}
}

void VertexBoneData::Reset()
{
	ZERO_MEM(IDs);
	ZERO_MEM(Weights);
}

void VertexBoneData::AddBoneData(int BoneID, float Weight)
{
	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	assert(0);
}

AnimationInfo::~AnimationInfo() 
{
}

AnimationInfo::AnimationInfo()
{
}

// Scene gets destroyed at the end of asset creation
// We need to copy animation related information
void AnimationInfo::SetScene(const aiScene * scene) 
{
	Animations.resize(scene->mNumAnimations);
	for (int x = 0, total = scene->mNumAnimations; x < total; ++x)
		Animations[x] = new aiAnimation(*scene->mAnimations[x]);

	RootNode = new aiNode(*scene->mRootNode);
}

size_t AnimationInfo::NumAnimations() const 
{
	return Animations.size(); 
}

Asset_Model::~Asset_Model()
{
	if (ExistsYet())
		glDeleteBuffers(7, buffers);	
	if (m_fence != nullptr)
		glDeleteSync(m_fence);
}

Asset_Model::Asset_Model(const string & filename) : Asset(filename)
{
	mesh_size = 0;
	bbox_min = vec3(0.0f);
	bbox_max = vec3(0.0f);
	for each (auto &buffer in buffers)
		buffer = -1;
	m_fence = nullptr;
}

int Asset_Model::GetAssetType()
{
	return ASSET_TYPE;
}

bool Asset_Model::ExistsYet()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (Asset::ExistsYet() && m_fence != nullptr ) {
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

GLuint Asset_Model::GenerateVAO()
{
	GLuint vaoID = 0;

	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
	for (unsigned int x = 0; x < NUM_VERTEX_ATTRIBUTES; ++x)
		glEnableVertexAttribArray(x);
	glBindVertexArray(0);

	return vaoID;
}

void Asset_Model::UpdateVAO(const GLuint & vaoID)
{
	shared_lock<shared_mutex> guard(m_mutex);
	glBindVertexArray(vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

	glBindVertexArray(0);
}

GLuint Asset_Model::GetSkinID(const unsigned int & desired)
{
	shared_lock<shared_mutex> guard(m_mutex);
	return skins[max(0, min(skins.size() - 1, desired))]->mat_spot;
}

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Uses hardcoded values
void fetchDefaultAsset(Shared_Asset_Model & asset)
{	
	// Check if a copy already exists
	if (Asset_Manager::QueryExistingAsset<Asset_Model>(asset, "defaultModel"))
		return;

	// Create hardcoded alternative
	Asset_Manager::CreateNewAsset<Asset_Model>(asset, "defaultModel");
	asset->data.vs = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->data.uv= vector<vec2>{ vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 0), vec2(1, 1), vec2(0, 1) };
	asset->data.nm = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->data.tg = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->data.bt = vector<vec3>{ vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(-1, 1, 0) };
	asset->bbox_min = vec3(-1);
	asset->bbox_max = vec3(1);
	asset->data.bones.resize(6);
	asset->skins.resize(1);
	Asset_Loader::load_asset(asset->skins[0], "defaultMaterial");
	Asset_Manager::AddWorkOrder(new Model_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Model & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::QueryExistingAsset<Asset_Model>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = DIRECTORY_MODEL + filename;
		if (!FileReader::FileExistsOnDisk(fullDirectory)) {
			MSG::Error(FILE_MISSING, fullDirectory);
			fetchDefaultAsset(user);
			return;
		}

		// Create the asset
		Asset_Manager::CreateNewAsset<Asset_Model, Model_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

// Calculates a Axis Aligned Bounding Box from a set of vertices. 
// Returns it as updated minimum and maximum values &minOut and &maxOut respectively
void calculate_AABB(const vector<vec3> & vertices, vec3 & minOut, vec3 & maxOut)
{
	if (vertices.size() >= 1) {
		const vec3 &vector = vertices.at(0);
		float minX = vector.x, maxX = vector.x, minY = vector.y, maxY = vector.y, minZ = vector.z, maxZ = vector.z;
		for (int x = 1, total = vertices.size(); x < total; ++x)
		{
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
	}
}

void Model_WorkOrder::Initialize_Order()
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
		MSG::Error(FILE_CORRUPT, m_filename);
		fetchDefaultAsset(m_asset);
		return;
	}

	unique_lock<shared_mutex> m_asset_guard(m_asset->m_mutex);
	GeometryInfo &gi = m_asset->data;

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
	m_asset->animationInfo.SetScene(scene);
	m_asset->mesh_size = gi.vs.size(); // Final vertex size (needed for draw arrays call)
	gi.bones.resize(gi.vs.size());
	calculate_AABB(gi.vs, m_asset->bbox_min, m_asset->bbox_max);
	Initialize_Bones(m_asset, scene);

	// Generate all the required skins
	m_asset->skins.resize(max(1, (scene->mNumMaterials - 1)));
	if (scene->mNumMaterials > 1) 		
		for (int x = 1; x < scene->mNumMaterials; ++x) // ignore scene material [0] 
			Generate_Material(m_asset->skins[x - 1], scene->mMaterials[x]);	
	else
		Generate_Material(m_asset->skins[0]);
}

void Model_WorkOrder::Finalize_Order()
{
	if (!m_asset->ExistsYet()) {
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		
		auto &data = m_asset->data;
		auto &buffers = m_asset->buffers;
		const size_t &arraySize = data.vs.size();

		glGenBuffers(6, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.vs[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.nm[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.tg[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.bt[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec2), &data.uv[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(VertexBoneData), &data.bones[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);		
		m_asset->m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();

		write_guard.unlock();
		write_guard.release();
		m_asset->Finalize();
	}
}
mat4 inline aiMatrixtoMat4x4(const aiMatrix4x4 &d)
{
	return mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

void Model_WorkOrder::Initialize_Bones(Shared_Asset_Model & model, const aiScene * scene)
{
	vector<BoneInfo> &boneInfo = model->animationInfo.meshTransforms;
	map<string, int> &m_BoneMapping = model->animationInfo.boneMap;
	vector<VertexBoneData> &bones = model->data.bones;

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

void Model_WorkOrder::Generate_Material(Shared_Asset_Material & modelMaterial, const aiMaterial * sceneMaterial)
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

void Model_WorkOrder::Generate_Material(Shared_Asset_Material & modelMaterial)
{
	std::string materialFilename = m_filename.substr(m_filename.find("Models\\"));
	materialFilename = materialFilename.substr(0, materialFilename.find_first_of("."));
	Asset_Loader::load_asset(modelMaterial, materialFilename);
}
