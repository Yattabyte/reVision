#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/version.h"
#include <atomic>
#include <algorithm>
#include <string>
#include <cctype>


constexpr size_t MAX_IMPORTERS = 4;
struct Importer_Pool {
	Importer_Pool() noexcept {
		std::generate(pool, pool + MAX_IMPORTERS, []() {return new Assimp::Importer(); });
	}

	/** Borrow a single importer.
	@return		returns a single importer*/
	[[nodiscard]] Assimp::Importer* rentImporter() noexcept {
		// Check if any of our importers are free to be used
		std::unique_lock<std::mutex> readGuard(poolMutex);
		if (available != 0ULL)
			return pool[--available];
		// Otherwise create a new one
		return new Assimp::Importer();
	}

	void returnImporter(Assimp::Importer* returnedImporter) noexcept {
		// Check if we have enough importers, free extra
		std::unique_lock<std::mutex> readGuard(poolMutex);
		if (available >= 4)
			delete returnedImporter;
		else
			pool[available++] = returnedImporter;
	}


private:
	Assimp::Importer* pool[MAX_IMPORTERS]{};
	size_t available = MAX_IMPORTERS;
	std::mutex poolMutex;
};

static Importer_Pool importer_pool;

/** Convert an aiMatrix to glm::mat4.
@param	d	the aiMatrix to convert from
@return		the glm::mat4 converted to */
inline glm::mat4 aiMatrix_to_Mat4x4(const aiMatrix4x4& d) noexcept
{
	return glm::mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

[[nodiscard]] inline Node* copy_node(const aiNode* oldNode) noexcept
{
	Node* newNode = new Node(std::string(oldNode->mName.data), aiMatrix_to_Mat4x4(oldNode->mTransformation));
	// Copy Children
	newNode->children.resize(oldNode->mNumChildren);
	for (unsigned int c = 0; c < oldNode->mNumChildren; ++c)
		newNode->children[c] = copy_node(oldNode->mChildren[c]);
	return newNode;
}

bool Mesh_IO::Import_Model(Engine* engine, const std::string& relativePath, Mesh_Geometry& importedData) noexcept
{
	// Check if the file exists
	if (!Engine::File_Exists(relativePath)) {
		engine->getManager_Messages().error("The file \"" + relativePath + "\" does not exist.");
		return false;
	}

	// Get Importer Resource
	Assimp::Importer* importer = importer_pool.rentImporter();
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
	if (scene == nullptr) {
		engine->getManager_Messages().error("The file \"" + relativePath + "\" exists, but is corrupted.");
		return false;
	}

	// Import geometry
	for (unsigned int a = 0, atotal = scene->mNumMeshes; a < atotal; ++a) {
		const aiMesh* mesh = scene->mMeshes[a];
		const GLuint meshMaterialOffset = std::max(0U, scene->mNumMaterials > 1 ? mesh->mMaterialIndex - 1U : 0U);
		for (unsigned int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const aiFace& face = mesh->mFaces[x];
			for (unsigned int b = 0, indCount = face.mNumIndices; b < indCount; ++b) {
				const auto& index = face.mIndices[b];
				importedData.vertices.emplace_back(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);

				const auto normal = mesh->HasNormals() ? mesh->mNormals[index] : aiVector3D(1.0F, 1.0F, 1.0F);
				importedData.normals.push_back(glm::normalize(glm::vec3(normal.x, normal.y, normal.z)));

				const auto tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[index] : aiVector3D(1.0F, 1.0F, 1.0F);
				importedData.tangents.push_back(glm::normalize(glm::vec3(tangent.x, tangent.y, tangent.z)));

				const auto bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[index] : aiVector3D(1.0F, 1.0F, 1.0F);
				importedData.bitangents.push_back(glm::normalize(glm::vec3(bitangent.x, bitangent.y, bitangent.z)));

				const auto uvmap = mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][index]) : aiVector3D(0, 0, 0);
				importedData.texCoords.emplace_back(uvmap.x, uvmap.y);

				importedData.materialIndices.push_back(meshMaterialOffset);
				importedData.meshIndices.push_back(a);
			}
		}
	}

	// Copy Animations
	importedData.animations.resize(scene->mNumAnimations);
	for (int a = 0, total = scene->mNumAnimations; a < total; ++a) {
		auto* animation = scene->mAnimations[a];
		importedData.animations[a] = Animation(animation->mNumChannels, animation->mTicksPerSecond, animation->mDuration);

		// Copy Channels
		importedData.animations[a].channels.resize(animation->mNumChannels);
		for (unsigned int c = 0; c < scene->mAnimations[a]->mNumChannels; ++c) {
			auto* channel = scene->mAnimations[a]->mChannels[c];
			importedData.animations[a].channels[c] = new Node_Animation(std::string(channel->mNodeName.data));

			// Copy Keys
			importedData.animations[a].channels[c]->scalingKeys.resize(channel->mNumScalingKeys);
			for (unsigned int n = 0; n < channel->mNumScalingKeys; ++n) {
				auto& key = channel->mScalingKeys[n];
				importedData.animations[a].channels[c]->scalingKeys[n] = Animation_Time_Key<glm::vec3>(key.mTime, glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
			}
			importedData.animations[a].channels[c]->rotationKeys.resize(channel->mNumRotationKeys);
			for (unsigned int n = 0; n < channel->mNumRotationKeys; ++n) {
				auto& key = channel->mRotationKeys[n];
				importedData.animations[a].channels[c]->rotationKeys[n] = Animation_Time_Key<glm::quat>(key.mTime, glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
			}
			importedData.animations[a].channels[c]->positionKeys.resize(channel->mNumPositionKeys);
			for (unsigned int n = 0; n < channel->mNumPositionKeys; ++n) {
				auto& key = channel->mPositionKeys[n];
				importedData.animations[a].channels[c]->positionKeys[n] = Animation_Time_Key<glm::vec3>(key.mTime, glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
			}
		}
	}

	// Copy Root Node and bones
	importedData.rootNode = copy_node(scene->mRootNode);
	importedData.bones.resize(importedData.vertices.size());
	int vertexOffset = 0;
	for (auto a = 0U, atotal = scene->mNumMeshes; a < atotal; ++a) {
		const aiMesh* mesh = scene->mMeshes[a];

		for (auto b = 0U, numBones = mesh->mNumBones; b < numBones; ++b) {
			size_t BoneIndex = 0;
			std::string BoneName(mesh->mBones[b]->mName.data);

			if (importedData.boneMap.find(BoneName) == importedData.boneMap.end()) {
				BoneIndex = importedData.boneTransforms.size();
				importedData.boneTransforms.emplace_back(1.0F);
			}
			else
				BoneIndex = importedData.boneMap[BoneName];

			importedData.boneMap[BoneName] = BoneIndex;
			importedData.boneTransforms[BoneIndex] = aiMatrix_to_Mat4x4(mesh->mBones[b]->mOffsetMatrix);

			for (auto j = 0U, numWeights = mesh->mBones[b]->mNumWeights; j < numWeights; ++j) {
				int VertexID = vertexOffset + mesh->mBones[b]->mWeights[j].mVertexId;
				float Weight = mesh->mBones[b]->mWeights[j].mWeight;
				importedData.bones[VertexID].AddBoneData((int)BoneIndex, Weight);
			}
		}

		for (auto x = 0U, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const auto& face = mesh->mFaces[x];
			for (auto b = 0U, indCount = face.mNumIndices; b < indCount; ++b)
				vertexOffset++;
		}
	}

	// Copy Texture Paths
	if (scene->mNumMaterials > 1U)
		for (auto x = 1U; x < scene->mNumMaterials; ++x) {
			constexpr static auto getMaterial = [](const aiScene* scene, const unsigned int& materialIndex) -> Material_Strings {
				constexpr static auto getTexture = [](const aiMaterial* sceneMaterial, const aiTextureType& textureType, std::string& texturePath) {
					aiString path;
					for (unsigned int x = 0; x < sceneMaterial->GetTextureCount(textureType); ++x) {
						if (sceneMaterial->GetTexture(textureType, x, &path) == AI_SUCCESS) {
							texturePath = path.C_Str();
							return;
						}
					}
				};
				//std::string albedo = "albedo.png", normal = "normal.png", metalness = "metalness.png", roughness = "roughness.png", height = "height.png", AO = "ao.png";
				std::string albedo, normal, metalness, roughness, height, ao;
				if (materialIndex >= 0) {
					const auto* sceneMaterial = scene->mMaterials[materialIndex];
					getTexture(sceneMaterial, aiTextureType_DIFFUSE, albedo);
					getTexture(sceneMaterial, aiTextureType_NORMALS, normal);
					getTexture(sceneMaterial, aiTextureType_SPECULAR, metalness);
					getTexture(sceneMaterial, aiTextureType_SHININESS, roughness);
					getTexture(sceneMaterial, aiTextureType_HEIGHT, height);
					getTexture(sceneMaterial, aiTextureType_AMBIENT, ao);

					constexpr static auto findStringIC = [](const std::string& strHaystack, const std::string& strNeedle) {
						auto it = std::search(
							strHaystack.begin(), strHaystack.end(),
							strNeedle.begin(), strNeedle.end(),
							[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
						);
						return (it != strHaystack.end());
					};
					if (normal.empty() && findStringIC(height, "normal")) {
						auto temp = normal;
						normal = height;
						height = temp;
					}
				}
				return Material_Strings(albedo, normal, metalness, roughness, height, ao);
			};

			// Import Mesh Material
			importedData.materials.push_back(getMaterial(scene, x));
		}
	else
		//importedData.materials.push_back(Material_Strings("albedo.png", "normal.png", "metalness.png", "roughness.png", "height.png", "ao.png"));
		importedData.materials.emplace_back("", "", "", "", "", "");

	// Free Importer Resource
	importer_pool.returnImporter(importer);
	return true;
}

std::string Mesh_IO::Get_Version() noexcept
{
	return std::to_string(aiGetVersionMajor()) + "." + std::to_string(aiGetVersionMinor()) + "." + std::to_string(aiGetVersionRevision());
}

VertexBoneData::VertexBoneData() noexcept
{
	Reset();
}

VertexBoneData::VertexBoneData(const VertexBoneData& vbd) noexcept
{
	Reset();
	for (size_t i = 0; i < 4; ++i) {
		IDs[i] = vbd.IDs[i];
		Weights[i] = vbd.Weights[i];
	}
}

inline void VertexBoneData::Reset() noexcept
{
	memset(IDs, 0, sizeof(IDs));
	memset(Weights, 0, sizeof(Weights));
}

inline void VertexBoneData::AddBoneData(const int& BoneID, const float& Weight) noexcept
{
	for (size_t i = 0; i < 4; ++i)
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	assert(0);
}