#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"
#include "assimp\version.h"
#include <atomic>


constexpr size_t MAX_IMPORTERS = 4;
struct Importer_Pool {
	Assimp::Importer* pool[MAX_IMPORTERS];
	std::atomic_size_t available = MAX_IMPORTERS;

	Importer_Pool() {
		std::generate(pool, pool + MAX_IMPORTERS, [](){return new Assimp::Importer(); });
	}

	/** Borrow a single importer. 
	@return		returns a single importer*/
	Assimp::Importer * rentImporter() {
		// Check if any of our importers are free to be used
		if (available) 
			return pool[--available];		
		// Otherwise create a new one
		else
			return new Assimp::Importer();
	}

	void returnImporter(Assimp::Importer * returnedImporter) {
		// Check if we have enough importers, free extra
		if (available >= 4)
			delete returnedImporter;
		else 
			pool[available++] = returnedImporter;		
	}
};

static Importer_Pool importer_pool;

/** Convert an aiMatrix to glm::mat4.
@param	d	the aiMatrix to convert from
@return		the glm::mat4 converted to */ 
inline glm::mat4 aiMatrix_to_Mat4x4(const aiMatrix4x4 &d)
{
	return glm::mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

inline Node * copy_node(const aiNode * oldNode) 
{
	Node * newNode = new Node(std::string(oldNode->mName.data), aiMatrix_to_Mat4x4(oldNode->mTransformation));
	// Copy Children
	newNode->children.resize(oldNode->mNumChildren);
	for (unsigned int c = 0; c < oldNode->mNumChildren; ++c) 
		newNode->children[c] = copy_node(oldNode->mChildren[c]);
	return newNode;
}

bool Mesh_IO::Import_Model(Engine * engine, const std::string & relativePath, Mesh_Geometry & data_container)
{
	// Check if the file exists
	if (!Engine::File_Exists(relativePath)) {
		engine->reportError(MessageManager::FILE_MISSING, relativePath);
		return false;
	}

	// Get Importer Resource
	Assimp::Importer * importer = importer_pool.rentImporter();
	const aiScene* scene = importer->ReadFile(
		Engine::Get_Current_Dir() + relativePath, 
		aiProcess_LimitBoneWeights |
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace /*|
		aiProcess_SplitLargeMeshes |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph |
		aiProcess_GenSmoothNormals
		aiProcess_ImproveCacheLocality*/
	);

	// Check if scene imported successfully
	if (!scene) {
		engine->reportError(MessageManager::FILE_CORRUPT, relativePath);
		return false;
	}

	// Import geometry
	for (int a = 0, atotal = scene->mNumMeshes; a < atotal; ++a) {
		const aiMesh * mesh = scene->mMeshes[a];
		const GLuint meshMaterialOffset = std::max(0u, scene->mNumMaterials > 1 ? mesh->mMaterialIndex - 1u : 0u) * 3u;
		for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const aiFace & face = mesh->mFaces[x];
			for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b) {
				const int & index = face.mIndices[b];
				data_container.vertices.push_back(glm::vec3(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z));

				const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[index] : aiVector3D(1.0f, 1.0f, 1.0f);
				data_container.normals.push_back(glm::normalize(glm::vec3(normal.x, normal.y, normal.z)));

				const aiVector3D tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[index] : aiVector3D(1.0f, 1.0f, 1.0f);
				data_container.tangents.push_back(glm::normalize(glm::vec3(tangent.x, tangent.y, tangent.z)));

				const aiVector3D bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[index] : aiVector3D(1.0f, 1.0f, 1.0f);
				data_container.bitangents.push_back(glm::normalize(glm::vec3(bitangent.x, bitangent.y, bitangent.z)));

				const aiVector3D uvmap = mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][index]) : aiVector3D(0, 0, 0);
				data_container.texCoords.push_back(glm::vec2(uvmap.x, uvmap.y));

				data_container.materialIndices.push_back(meshMaterialOffset);
			}

		}
	}
	
	// Import animations
	// Copy Animations
	data_container.animations.resize(scene->mNumAnimations);
	for (int a = 0, total = scene->mNumAnimations; a < total; ++a) {
		auto * animation = scene->mAnimations[a];
		data_container.animations[a] = Animation(animation->mNumChannels, animation->mTicksPerSecond, animation->mDuration);

		// Copy Channels
		data_container.animations[a].channels.resize(animation->mNumChannels);
		for (unsigned int c = 0; c < scene->mAnimations[a]->mNumChannels; ++c) {
			auto * channel = scene->mAnimations[a]->mChannels[c];
			data_container.animations[a].channels[c] = new Node_Animation(std::string(channel->mNodeName.data));

			// Copy Keys
			data_container.animations[a].channels[c]->scalingKeys.resize(channel->mNumScalingKeys);
			for (unsigned int n = 0; n < channel->mNumScalingKeys; ++n) {
				auto & key = channel->mScalingKeys[n];
				data_container.animations[a].channels[c]->scalingKeys[n] = Animation_Time_Key<glm::vec3>(key.mTime, glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
			}
			data_container.animations[a].channels[c]->rotationKeys.resize(channel->mNumRotationKeys);
			for (unsigned int n = 0; n < channel->mNumRotationKeys; ++n) {
				auto & key = channel->mRotationKeys[n];
				data_container.animations[a].channels[c]->rotationKeys[n] = Animation_Time_Key<glm::quat>(key.mTime, glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
			}
			data_container.animations[a].channels[c]->positionKeys.resize(channel->mNumPositionKeys);
			for (unsigned int n = 0; n < channel->mNumPositionKeys; ++n) {
				auto & key = channel->mPositionKeys[n];
				data_container.animations[a].channels[c]->positionKeys[n] = Animation_Time_Key<glm::vec3>(key.mTime, glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
			}
		}
	}

	// Copy Root Node and bones
	data_container.rootNode = copy_node(scene->mRootNode);
	data_container.bones.resize(data_container.vertices.size());
	int vertexOffset = 0;
	for (int a = 0, atotal = scene->mNumMeshes; a < atotal; ++a) {
		const aiMesh * mesh = scene->mMeshes[a];

		for (int B = 0, numBones = mesh->mNumBones; B < numBones; B++) {
			size_t BoneIndex = 0;
			std::string BoneName(mesh->mBones[B]->mName.data);

			if (data_container.boneMap.find(BoneName) == data_container.boneMap.end()) {
				BoneIndex = data_container.boneTransforms.size();
				data_container.boneTransforms.push_back(glm::mat4(1.0f));
			}
			else
				BoneIndex = data_container.boneMap[BoneName];

			data_container.boneMap[BoneName] = BoneIndex;
			data_container.boneTransforms[BoneIndex] = aiMatrix_to_Mat4x4(mesh->mBones[B]->mOffsetMatrix);

			for (unsigned int j = 0; j < mesh->mBones[B]->mNumWeights; j++) {
				int VertexID = vertexOffset + mesh->mBones[B]->mWeights[j].mVertexId;
				float Weight = mesh->mBones[B]->mWeights[j].mWeight;
				data_container.bones[VertexID].AddBoneData((int)BoneIndex, Weight);
			}
		}

		for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const aiFace& face = mesh->mFaces[x];
			for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b)
				vertexOffset++;
		}
	}


	// Import Materials
	if (scene->mNumMaterials > 1)
		for (int x = 1, total = scene->mNumMaterials; x < total; ++x) {
			auto * sceneMaterial = scene->mMaterials[x];
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
				size_t exspot = minusD.find_last_of(".");
				extension = minusD.substr(exspot, minusD.length());
				size_t diffuseStart = minusD.find("diff");
				if (diffuseStart != std::string::npos)
					minusD = minusD.substr(0, diffuseStart);
				else
					minusD = minusD.substr(0, exspot) + "_";
				templateTexture = minusD;
			}

			// Importer might not distinguish between height and normal maps
			if (normal_exists != AI_SUCCESS && height_exists == AI_SUCCESS) {
				std::string norm_string(height.C_Str());
				const size_t norm_spot = norm_string.find_last_of("norm");
				if (norm_spot != std::string::npos) {
					// Normal map confirmed to be in height map spot, move it over
					normal = height;
					normal_exists = AI_SUCCESS;
					height_exists = AI_FAILURE;
				}
			}

			data_container.materials.push_back(Material(
				(albedo_exists == AI_SUCCESS ? albedo.C_Str() : "albedo.png"),
				(normal_exists == AI_SUCCESS ? normal.C_Str() : templateTexture + "normal" + extension),
				(metalness_exists == AI_SUCCESS ? metalness.C_Str() : templateTexture + "metalness" + extension),
				(roughness_exists == AI_SUCCESS ? roughness.C_Str() : templateTexture + "roughness" + extension),
				(height_exists == AI_SUCCESS ? height.C_Str() : templateTexture + "height" + extension),
				(ao_exists == AI_SUCCESS ? ao.C_Str() : templateTexture + "ao" + extension)
			));
		}
	else {
		data_container.materials.push_back(Material(
			 "albedo.png", "normal.png", "metalness.png", "roughness.png", "height.png", "ao.png"
		));
	}


	// Free Importer Resource
	importer_pool.returnImporter(importer);
	return true;
}

const std::string Mesh_IO::Get_Version()
{
	return std::to_string(aiGetVersionMajor()) + "." + std::to_string(aiGetVersionMinor()) + "." + std::to_string(aiGetVersionRevision());
}

VertexBoneData::~VertexBoneData() {}

VertexBoneData::VertexBoneData() 
{
	Reset();
}

VertexBoneData::VertexBoneData(const VertexBoneData & vbd) 
{
	Reset();
	for (size_t i = 0; i < 4; ++i) {
		IDs[i] = vbd.IDs[i];
		Weights[i] = vbd.Weights[i];
	}
}

inline void VertexBoneData::Reset() 
{
	memset(IDs, 0, sizeof(IDs));
	memset(Weights, 0, sizeof(Weights));
}

inline void VertexBoneData::AddBoneData(const int & BoneID, const float & Weight) 
{
	for (size_t i = 0; i < 4; ++i)
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	assert(0);
}
