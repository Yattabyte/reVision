#pragma once
#ifndef	MODEL_IO_H
#define	MODEL_IO_H
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
struct Material {
	string albedo = "", normal = "", metalness = "", roughness = "", height = "", ao = "";
	Material(const string & al = "albedo" , const string & n = "normal", const string & m = "metalness", const string & r = "roughness", const string & h = "height", const string & a = "ao")
		: albedo(al), normal(n), metalness(m), roughness(r), height(h), ao(a) {}
};
struct Model_Geometry {
	// Per Vertex Attributes
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec3> tangents;
	vector<vec3> bitangents;
	vector<vec2> texCoords;

	// Materials
	vector<Material> materials;

	// Animation
	vector<VertexBoneData> bones;
	vector<BoneTransform> boneTransforms;
	map<string, int> boneMap;
	vector<aiAnimation*> animations;
	aiNode * rootNode;
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
	import_model		= 0b0111'1111,
	import_all			= 0b1111'1111
};

/** 
 * A static helper class used for reading/writing models.
 * Uses the Assimp library: http://assimp.sourceforge.net/
 **/
class Model_IO
{
public:
	/** Import a model from disk.
	 * @param	engine			the engine to import to
	 * @param	fulldirectory	the path to the file
	 * @param	importFlags		bitflags directing how to import the model
	 * @param	importedData	the container to place the imported data within
	 * @return					true on successfull import, false otherwise (error reported to engine) */
	static bool Import_Model(Engine * engine, const string & fulldirectory, const unsigned int & importFlags, Model_Geometry & importedData);
};

#endif // MODEL_IO_H