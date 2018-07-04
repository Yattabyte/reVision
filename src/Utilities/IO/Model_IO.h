#pragma once
#ifndef	MODELIO_H
#define	MODELIO_H
#define NUM_BONES_PER_VEREX 4
#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace glm;
class Engine;
class aiAnimation;
class aiNode;
class aiMaterial;

struct VertexBoneData {
	int IDs[NUM_BONES_PER_VEREX];
	float Weights[NUM_BONES_PER_VEREX];


	~VertexBoneData();
	VertexBoneData();
	VertexBoneData(const VertexBoneData & vbd);
	void Reset();
	void AddBoneData(const int & BoneID, const float & Weight);
};
struct BoneTransform { mat4 offset, final; };
struct Model_Geometry {
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec3> tangents;
	vector<vec3> bitangents;
	vector<vec2> texCoords;
	vector<VertexBoneData> bones;
	vector<BoneTransform> boneTransforms;
	map<string, int> boneMap;
	vector<aiAnimation*> animations;
	aiNode * rootNode;
	vector<aiMaterial*> materials;
};

/* Import Directives */
enum Model_IO_Flags {
	import_vertices		= 0b0000'0001,
	import_normals		= 0b0000'0010,
	import_tangents		= 0b0000'0100,
	import_bitangents	= 0b0000'1000,
	import_texcoords	= 0b0001'0000,
	import_animation	= 0b0010'0000,
	import_materials	= 0b0100'0000,

	import_hull			= 0b0000'0001,
	import_primitive	= 0b0001'0001,
	import_model		= 0b0111'1111
};

/** 
 * A static helper class used for reading/writing models.
 * Uses the Assimp library: http://assimp.sourceforge.net/
 **/
class Model_IO
{
public:
	static bool Import_Model(Engine * engine, const string & fulldirectory, const unsigned int & importFlags, Model_Geometry & importedData);
};

#endif // MODELIMPORTER_H