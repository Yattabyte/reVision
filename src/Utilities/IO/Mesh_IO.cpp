#include "Utilities/IO/Mesh_IO.h"
#include "Engine.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/version.h"
#include "glm/gtc/matrix_transform.hpp"
#include <atomic>
#include <algorithm>
#include <string>
#include <cctype>


/** Convert an aiMatrix to glm::mat4.
@param	d	the aiMatrix to convert from.
@return		the glm::mat4 converted to. */
inline glm::mat4 aiMatrix_to_Mat4x4(const aiMatrix4x4& d) noexcept
{
	return glm::mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

/** Copy a node into a new node. 
@param	oldNode		the old node to copy from.
@return				the new node. */
[[nodiscard]] inline Node copy_node(const aiNode* oldNode)
{
	// Copy Node
	Node newNode(std::string(oldNode->mName.data), aiMatrix_to_Mat4x4(oldNode->mTransformation));

	// Copy Node's Children
	const auto childCount = oldNode->mNumChildren;
	newNode.children.resize(childCount);
	for (unsigned int c = 0; c < childCount; ++c)
		newNode.children[c] = copy_node(oldNode->mChildren[c]);
	return newNode;
}

bool Mesh_IO::Import_Model(Engine& engine, const std::string& relativePath, Mesh_Geometry& importedData)
{
	// Check if the file exists
	if (!Engine::File_Exists(relativePath)) {
		engine.getManager_Messages().error("The file \"" + relativePath + "\" does not exist.");
		return false;
	}

	// Get Importer Resource
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
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
		engine.getManager_Messages().error("The file \"" + relativePath + "\" exists, but is corrupted.");
		return false;
	}

	// Import geometry
	const auto meshCount = scene->mNumMeshes;
	const auto materialCount = scene->mNumMaterials;
	for (unsigned int a = 0; a < meshCount; ++a) {
		const aiMesh* mesh = scene->mMeshes[a];
		const GLuint meshMaterialOffset = std::max(0U, materialCount > 1 ? mesh->mMaterialIndex - 1U : 0U);
		const auto faceCount = mesh->mNumFaces;
		for (unsigned int x = 0; x < faceCount; ++x) {
			const aiFace& face = mesh->mFaces[x];
			const auto indCount = face.mNumIndices;
			for (unsigned int b = 0; b < indCount; ++b) {
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
	const auto animationCount = scene->mNumAnimations;
	importedData.animations.resize(animationCount);
	for (unsigned int a = 0; a < animationCount; ++a) {
		const auto* animation = scene->mAnimations[a];
		importedData.animations[a] = Animation(animation->mNumChannels, animation->mTicksPerSecond, animation->mDuration);

		// Copy Channels
		const auto channelCount = animation->mNumChannels;
		importedData.animations[a].channels.resize(channelCount);
		for (unsigned int c = 0; c < channelCount; ++c) {
			const auto* channel = animation->mChannels[c];
			importedData.animations[a].channels[c] = Node_Animation(std::string(channel->mNodeName.data));

			// Copy Keys
			const auto scalingKeyCount = channel->mNumScalingKeys;
			importedData.animations[a].channels[c].scalingKeys.resize(scalingKeyCount);
			for (unsigned int n = 0; n < scalingKeyCount; ++n) {
				const auto& key = channel->mScalingKeys[n];
				importedData.animations[a].channels[c].scalingKeys[n] = Animation_Time_Key<glm::vec3>(key.mTime, glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
			}
			const auto rotationKeyCount = channel->mNumRotationKeys;
			importedData.animations[a].channels[c].rotationKeys.resize(rotationKeyCount);
			for (unsigned int n = 0; n < rotationKeyCount; ++n) {
				const auto& key = channel->mRotationKeys[n];
				importedData.animations[a].channels[c].rotationKeys[n] = Animation_Time_Key<glm::quat>(key.mTime, glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
			}
			const auto positionKeyCount = channel->mNumPositionKeys;
			importedData.animations[a].channels[c].positionKeys.resize(positionKeyCount);
			for (unsigned int n = 0; n < positionKeyCount; ++n) {
				const auto& key = channel->mPositionKeys[n];
				importedData.animations[a].channels[c].positionKeys[n] = Animation_Time_Key<glm::vec3>(key.mTime, glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
			}
		}
	}

	// Copy Root Node and bones
	const auto VertexCount = importedData.vertices.size();
	importedData.rootNode = copy_node(scene->mRootNode);
	importedData.bones.resize(VertexCount);
	int vertexOffset = 0;
	for (unsigned int a = 0U; a < meshCount; ++a) {
		const aiMesh* mesh = scene->mMeshes[a];
		const auto boneCount = mesh->mNumBones;
		for (unsigned int b = 0U; b < boneCount; ++b) {
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
			
			const auto weightCount = mesh->mBones[b]->mNumWeights;
			for (unsigned int j = 0U; j < weightCount; ++j) {
				const int& VertexID = vertexOffset + mesh->mBones[b]->mWeights[j].mVertexId;
				const float& Weight = mesh->mBones[b]->mWeights[j].mWeight;
				if (VertexID < VertexCount)
					importedData.bones[VertexID].AddBoneData(static_cast<int>(BoneIndex), Weight);
			}
		}

		const auto faceCount = mesh->mNumFaces;
		for (unsigned int x = 0U; x < faceCount; ++x) {
			const auto& face = mesh->mFaces[x];
			const auto indexCount = face.mNumIndices;
			for (unsigned int b = 0U; b < indexCount; ++b)
				vertexOffset++;
		}
	}

	// Copy Texture Paths
	if (materialCount > 1U)
		for (auto x = 1U; x < materialCount; ++x) {
			constexpr static auto getMaterial = [](const aiScene& scene, const unsigned int& materialIndex) -> Material_Strings {
				constexpr static auto getTexture = [](const aiMaterial& sceneMaterial, const aiTextureType& textureType, std::string& texturePath) {
					for (unsigned int x = 0; x < sceneMaterial.GetTextureCount(textureType); ++x) {
						aiString path;
						if (sceneMaterial.GetTexture(textureType, x, &path) == AI_SUCCESS) {
							texturePath = path.C_Str();
							return;
						}
					}
				};
				//std::string albedo = "albedo.png", normal = "normal.png", metalness = "metalness.png", roughness = "roughness.png", height = "height.png", AO = "ao.png";
				std::string albedo;
				std::string normal;
				std::string metalness;
				std::string roughness;
				std::string height;
				std::string ao;
				if (materialIndex >= 0) {
					if (const auto* sceneMaterial = scene.mMaterials[materialIndex]) {
						getTexture(*sceneMaterial, aiTextureType_DIFFUSE, albedo);
						getTexture(*sceneMaterial, aiTextureType_NORMALS, normal);
						getTexture(*sceneMaterial, aiTextureType_SPECULAR, metalness);
						getTexture(*sceneMaterial, aiTextureType_SHININESS, roughness);
						getTexture(*sceneMaterial, aiTextureType_HEIGHT, height);
						getTexture(*sceneMaterial, aiTextureType_AMBIENT, ao);

						constexpr static auto findStringIC = [](const std::string& strHaystack, const std::string& strNeedle) {
							auto it = std::search(
								strHaystack.begin(), strHaystack.end(),
								strNeedle.begin(), strNeedle.end(),
								[](char ch1, char ch2) noexcept { return std::toupper(ch1) == std::toupper(ch2); }
							);
							return (it != strHaystack.end());
						};
						if (normal.empty() && findStringIC(height, "normal")) {
							auto temp = normal;
							normal = height;
							height = temp;
						}
					}
				}
				return Material_Strings(albedo, normal, metalness, roughness, height, ao);
			};

			// Import Mesh Material
			importedData.materials.push_back(getMaterial(*scene, x));
		}
	else
		//importedData.materials.push_back(Material_Strings("albedo.png", "normal.png", "metalness.png", "roughness.png", "height.png", "ao.png"));
		importedData.materials.emplace_back("", "", "", "", "", "");
	return true;
}

std::string Mesh_IO::Get_Version()
{
	return std::to_string(aiGetVersionMajor()) + "." + std::to_string(aiGetVersionMinor()) + "." + std::to_string(aiGetVersionRevision());
}

std::vector<std::string> Mesh_IO::Get_Supported_Types()
{
	aiString ext;
	Assimp::Importer importer;
	importer.GetExtensionList(ext);

	std::vector<std::string> extensions;
	std::string allExt(ext.C_Str());
	auto spot = allExt.find(';');
	while (spot != std::string::npos) {
		extensions.push_back(allExt.substr(1, spot - 1));
		allExt = allExt.substr(spot + 1, allExt.size() - spot);
		spot = allExt.find(';');
	}
	return extensions;
}

void VertexBoneData::Reset() noexcept
{
	for (auto i = 0; i < NUM_BONES_PER_VEREX; ++i) {
		IDs[i] = 0;
		Weights[i] = 0.0F;
	}
}

void VertexBoneData::AddBoneData(const int& BoneID, const float& Weight) noexcept
{
	for (auto i = 0; i < NUM_BONES_PER_VEREX; ++i)
		if (Weights[i] == 0.0F) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	assert(0);
}