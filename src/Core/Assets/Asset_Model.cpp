#include "Assets\Asset_Model.h"
#include "Managers\Message_Manager.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 4

using namespace Asset_Manager;

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
	if (finalized)
		glDeleteVertexArrays(1, &gl_vao_ID);
}

Asset_Model::Asset_Model()
{
	gl_vao_ID = 0;
	mesh_size = 0;
	filename = "";
	bbox_min = vec3(0.0f);
	bbox_max = vec3(0.0f);
	finalized = false;
}

Asset_Model::Asset_Model(const string & _filename) : Asset_Model()
{
	filename = _filename;
}

int Asset_Model::GetAssetType()
{
	return ASSET_TYPE;
}

void Asset_Model::Finalize()
{
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (!finalized) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_mutex);
		GLuint buffers[7];
		const size_t &arraySize = data.vs.size();
		glGenVertexArrays(1, &gl_vao_ID);
		glBindVertexArray(gl_vao_ID);
		glGenBuffers(7, buffers);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.vs[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.nm[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.tg[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec3), &data.bt[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(vec2), &data.uv[0][0], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(GLuint), &data.ts[0], GL_STATIC_DRAW);
		glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
		glBufferData(GL_ARRAY_BUFFER, arraySize * sizeof(VertexBoneData), &data.bones[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(6);
		glVertexAttribIPointer(6, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

		glBindVertexArray(0);
		glDeleteBuffers(7, buffers);
		finalized = true;
	}
}

Shared_Asset_Model fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	std::map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Model::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Model::GetAssetType()];
	if (default_asset.get() == nullptr)
		default_asset = shared_ptr<Asset_Model>(new Asset_Model());
	return dynamic_pointer_cast<Asset_Model>(default_asset);
}

void calculate_AABB(const vector<vec3> &vertices, vec3 &minOut, vec3 &maxOut)
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

void initialize_Model_Bones(Shared_Asset_Model &model, const aiScene* scene)
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
			boneInfo[BoneIndex].BoneOffset = mesh->mBones[B]->mOffsetMatrix;

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

void initialize_Model_Material(Shared_Asset_Material &texture, const aiMesh * mesh, const aiMaterial * material, const string & specificTexDir)
{
	// Get the aiStrings for all the textures for a material
	aiString	albedo, normal, metalness, roughness, height, ao;
	aiReturn	albedo_exists = material->GetTexture(aiTextureType_DIFFUSE, 0, &albedo),
				normal_exists = material->GetTexture(aiTextureType_NORMALS, 0, &normal),
				metalness_exists = material->GetTexture(aiTextureType_SPECULAR, 0, &metalness),
				roughness_exists = material->GetTexture(aiTextureType_SHININESS, 0, &roughness),
				height_exists = material->GetTexture(aiTextureType_HEIGHT, 0, &height),
				ao_exists = material->GetTexture(aiTextureType_AMBIENT, 0, &ao);

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
		/*ALBEDO*/						specificTexDir + (albedo_exists == AI_SUCCESS ? albedo.C_Str() : "diffuse.png"),
		/*NORMAL*/						specificTexDir + (normal_exists == AI_SUCCESS ? normal.C_Str() : templateTexture + "normal" + extension),
		/*METALNESS*/					specificTexDir + (metalness_exists == AI_SUCCESS ? metalness.C_Str() : templateTexture + "metalness" + extension),
		/*ROUGHNESS*/					specificTexDir + (roughness_exists == AI_SUCCESS ? roughness.C_Str() : templateTexture + "roughness" + extension),
		/*HEIGHT*/						specificTexDir + (height_exists == AI_SUCCESS ? height.C_Str() : templateTexture + "height" + extension),
		/*AO*/							specificTexDir + (ao_exists == AI_SUCCESS ? ao.C_Str() : templateTexture + "ao" + extension)
	};

	Asset_Manager::load_asset(texture, material_textures);
}

void initialize_Model(Shared_Asset_Model &model, const string &filename, bool *complete)
{
	int modelStart = filename.find("\\Models\\");
	std::string specificTexDir = std::string(filename).insert(modelStart, "\\Textures");
	specificTexDir = specificTexDir.substr(0, specificTexDir.find_last_of("\\") + 1);
	Assimp::Importer &importer = *new Assimp::Importer();
	const aiScene* scene = importer.ReadFile(filename,
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
		MSG::Error(FILE_CORRUPT, filename);
		model = fetchDefaultAsset();
		return;
	}

	{ // Separated as to destroy lock at completion
		unique_lock<shared_mutex> model_guard(model->m_mutex);
		GeometryInfo &gi = model->data;
		vector<Shared_Asset_Material> &textures = model->textures;
		textures.resize(scene->mNumMeshes);

		// Combine mesh data into single struct
		for (int a = 0, atotal = scene->mNumMeshes; a < atotal; a++) {

			const aiMesh *mesh = scene->mMeshes[a];
			Shared_Asset_Material &texture = textures[a];
			initialize_Model_Material(texture, mesh, scene->mMaterials[mesh->mMaterialIndex], specificTexDir);
			const GLuint &mat_spot = texture->mat_spot;

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
					gi.ts.push_back(mat_spot);
				}
			}
		}
		model->mesh_size = gi.vs.size(); // Final vertex size (needed for draw arrays call)
		calculate_AABB(gi.vs, model->bbox_min, model->bbox_max); // Bounding box size for frustum culling (only of model, we won't fs cull sub meshes)

		gi.bones.resize(gi.vs.size());
		initialize_Model_Bones(model, scene);
		model->animationInfo.SetScene(scene);
	}

	submitWorkorder(model);
	*complete = true;
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Model &user, const string &filename, const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_models = (fetchAssetList(Asset_Model::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (const auto &asset in assets_models) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Model derived_asset = dynamic_pointer_cast<Asset_Model>(asset);
				if (derived_asset) {
					if (derived_asset->filename == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						if (!threaded)
							user->Finalize();
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		const std::string &fulldirectory = getCurrentDir() + "\\Models\\" + filename;
		if (!fileOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultAsset();
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Model(new Asset_Model(filename));
			assets_models.push_back(user);
		}

		bool *complete = new bool(false);
		if (threaded) {
			thread *import_thread = new thread(initialize_Model, user, fulldirectory, complete);
			import_thread->detach();
			submitWorkthread(std::pair<thread*, bool*>(import_thread, complete));
		}
		else {
			initialize_Model(user, fulldirectory, complete);
			user->Finalize();
		}
	}
}