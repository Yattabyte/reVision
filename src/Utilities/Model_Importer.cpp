#include "Utilities\File_Reader.h"
#include "Utilities\Model_Importer.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"


int Model_Importer::import_Model(MessageManager & messageMananger, const string & fulldirectory, unsigned int pFlags, vector<btScalar> & points)
{
	// Check if the file exists
	if (!File_Reader::FileExistsOnDisk(fulldirectory)) {
		messageMananger.error(MessageManager::FILE_MISSING, fulldirectory);
		return 0;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fulldirectory, pFlags);
	// Check if the file is corrupted
	if (!scene) {
		messageMananger.error(MessageManager::FILE_CORRUPT, fulldirectory);
		return -1;
	}

	for (int a = 0, meshCount = scene->mNumMeshes; a < meshCount; ++a) {
		const aiMesh* mesh = scene->mMeshes[a];
		for (int b = 0, faceCount = mesh->mNumFaces; b < faceCount; ++b) {
			const aiFace& face = mesh->mFaces[b];
			for (int c = 0, indCount = face.mNumIndices; c < indCount; ++c) {
				const aiVector3D &aiv = mesh->mVertices[face.mIndices[c]];
				points.push_back(aiv.x);
				points.push_back(aiv.y);
				points.push_back(aiv.z);
			}
		}
	}
	return 1;
}

int Model_Importer::import_Model(MessageManager & messageMananger, const string & fulldirectory, unsigned int pFlags, vector<vec3> & vertices, vector<vec2> & uv_coords)
{
	// Check if the file exists
	if (!File_Reader::FileExistsOnDisk(fulldirectory)) {
		messageMananger.error(MessageManager::FILE_MISSING, fulldirectory);
		return 0;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fulldirectory, pFlags);
	// Check if the file is corrupted
	if (!scene) {
		messageMananger.error(MessageManager::FILE_CORRUPT, fulldirectory);
		return -1;
	}

	for (int a = 0, atotal = scene->mNumMeshes; a < atotal; a++) {
		const aiMesh *mesh = scene->mMeshes[a];
		for (int x = 0, faceCount = mesh->mNumFaces; x < faceCount; ++x) {
			const aiFace& face = mesh->mFaces[x];
			for (int b = 0, indCount = face.mNumIndices; b < indCount; ++b) {
				const int index = face.mIndices[b];
				const auto &vertex = mesh->mVertices[index];
				const auto &uvmap = mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][index]) : aiVector3D(0, 0, 0);
				vertices.push_back(vec3(vertex.x, vertex.y, vertex.z));
				uv_coords.push_back(vec2(uvmap.x, uvmap.y));
			}
		}
	}
	return 1;
}
