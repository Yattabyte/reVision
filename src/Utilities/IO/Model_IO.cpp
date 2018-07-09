#include "Utilities\IO\Model_IO.h"
#include "Utilities\File_Reader.h"
#include "Engine.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"
#include <shared_mutex>
#include <minmax.h>


struct Importer_Pool {
	shared_mutex pool_mutex;
	vector<Assimp::Importer*> pool;


	Importer_Pool() {
		for (int x = 0; x < 4; ++x)
			pool.push_back(new Assimp::Importer());
	}

	Assimp::Importer * rentImporter() {
		// Start Reading Pool
		unique_lock<shared_mutex> pool_readGuard(pool_mutex);

		// Check if any of our importers are free to be used
		if (pool.size()) {
			Assimp::Importer * freeImporter = pool.back();
			pool.pop_back();
			return freeImporter;
		}
		// Otherwise create a new one
		else
			return new Assimp::Importer();
	}

	void returnImporter(Assimp::Importer * returnedImporter) {
		// Start Reading Pool
		unique_lock<shared_mutex> pool_readGuard(pool_mutex);

		// Check if we have enough importers, free extra
		if (pool.size() >= 4)
			delete returnedImporter;
		else
			pool.push_back(returnedImporter);
	}
};

static Importer_Pool importer_pool;

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

bool Model_IO::Import_Model(Engine * engine, const string & fulldirectory, const unsigned int & importFlags, Model_Geometry & data_container)
{
	// Check if the file exists
	if (!File_Reader::FileExistsOnDisk(fulldirectory)) {
		engine->reportError(MessageManager::FILE_MISSING, fulldirectory);
		return false;
	}

	// Get Importer Resource
	Assimp::Importer * importer = importer_pool.rentImporter();
	const aiScene* scene = importer->ReadFile(
		fulldirectory, 
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
		engine->reportError(MessageManager::FILE_CORRUPT, fulldirectory);
		return false;
	}

	// Import geometry
	if (importFlags & (import_vertices | import_normals | import_tangents | import_bitangents | import_texcoords)) {
		for (int a = 0, atotal = scene->mNumMeshes; a < atotal; ++a) {
			const aiMesh * mesh = scene->mMeshes[a];
			for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
				const aiFace & face = mesh->mFaces[x];
				for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b) {
					const int & index = face.mIndices[b];
					if (importFlags & import_vertices)
						data_container.vertices.push_back(vec3(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z));
					if (importFlags & import_normals) {
						const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[index] : aiVector3D(1.0f, 1.0f, 1.0f);
						data_container.normals.push_back(glm::normalize(vec3(normal.x, normal.y, normal.z)));
					}
					if (importFlags & import_tangents) {
						const aiVector3D tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[index] : aiVector3D(1.0f, 1.0f, 1.0f);
						data_container.tangents.push_back(glm::normalize(vec3(tangent.x, tangent.y, tangent.z)));
					}
					if (importFlags & import_bitangents) {
						const aiVector3D bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[index] : aiVector3D(1.0f, 1.0f, 1.0f);
						data_container.bitangents.push_back(glm::normalize(vec3(bitangent.x, bitangent.y, bitangent.z)));
					}
					if (importFlags & import_texcoords) {
						const aiVector3D uvmap = mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][index]) : aiVector3D(0, 0, 0);
						data_container.texCoords.push_back(vec2(uvmap.x, uvmap.y));
					}
				}

			}
		}
	}

	// Import animations
	if (importFlags & import_animation) {
		data_container.animations.resize(scene->mNumAnimations);
		for (int x = 0, total = scene->mNumAnimations; x < total; ++x)
			data_container.animations[x] = new aiAnimation(*scene->mAnimations[x]);
		data_container.rootNode = new aiNode(*scene->mRootNode);
		data_container.bones.resize(data_container.vertices.size());
		int vertexOffset = 0;
		for (int a = 0, atotal = scene->mNumMeshes; a < atotal; ++a) {
			const aiMesh * mesh = scene->mMeshes[a];

			for (int B = 0, numBones = mesh->mNumBones; B < numBones; B++) {
				int BoneIndex = 0;
				string BoneName(mesh->mBones[B]->mName.data);

				if (data_container.boneMap.find(BoneName) == data_container.boneMap.end()) {
					BoneIndex = data_container.boneTransforms.size();
					data_container.boneTransforms.push_back(BoneTransform());
				}
				else
					BoneIndex = data_container.boneMap[BoneName];

				data_container.boneMap[BoneName] = BoneIndex;
				data_container.boneTransforms[BoneIndex].offset = aiMatrix_to_Mat4x4(mesh->mBones[B]->mOffsetMatrix);

				for (unsigned int j = 0; j < mesh->mBones[B]->mNumWeights; j++) {
					int VertexID = vertexOffset + mesh->mBones[B]->mWeights[j].mVertexId;
					float Weight = mesh->mBones[B]->mWeights[j].mWeight;
					data_container.bones[VertexID].AddBoneData(BoneIndex, Weight);
				}
			}

			for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
				const aiFace& face = mesh->mFaces[x];
				for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b)
					vertexOffset++;
			}
		}
	}

	// Import Materials
	if (importFlags & import_materials) {
		data_container.materials.resize(scene->mNumMaterials);
		for (int x = 0, total = scene->mNumMaterials; x < total; ++x)
			data_container.materials[x] = new aiMaterial(*scene->mMaterials[x]);
	}

	// Free Importer Resource
	importer_pool.returnImporter(importer);
	return true;
}

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

inline VertexBoneData::~VertexBoneData() {}

inline VertexBoneData::VertexBoneData() 
{
	Reset();
}

inline VertexBoneData::VertexBoneData(const VertexBoneData & vbd) 
{
	Reset();
	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
		IDs[i] = vbd.IDs[i];
		Weights[i] = vbd.Weights[i];
	}
}

inline void VertexBoneData::Reset() 
{
	ZERO_MEM(IDs);
	ZERO_MEM(Weights);
}

inline void VertexBoneData::AddBoneData(const int & BoneID, const float & Weight) 
{
	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	assert(0);
}
