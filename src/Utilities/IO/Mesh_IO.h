#pragma once
#ifndef	Mesh_IO_H
#define	Mesh_IO_H
#define NUM_BONES_PER_VEREX 4

#include "GL/glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;

struct VertexBoneData {
	int IDs[NUM_BONES_PER_VEREX];
	float Weights[NUM_BONES_PER_VEREX];
	~VertexBoneData() = default;
	VertexBoneData();
	VertexBoneData(const VertexBoneData & vbd);
	void Reset();
	void AddBoneData(const int & BoneID, const float & Weight);
};
struct Material {
	std::string albedo = "", normal = "", metalness = "", roughness = "", height = "", ao = "";
	Material(const std::string & al = "albedo" , const std::string & n = "normal", const std::string & m = "metalness", const std::string & r = "roughness", const std::string & h = "height", const std::string & a = "ao")
		: albedo(al), normal(n), metalness(m), roughness(r), height(h), ao(a) {}
};
template<typename T>
struct Animation_Time_Key {
	double time;
	T value;
	Animation_Time_Key() : time(0) {};
	Animation_Time_Key(const double & t, const T & v) : time(t), value(v) {};
};
struct Node_Animation {
	std::string nodeName;
	std::vector<Animation_Time_Key<glm::vec3>> scalingKeys;
	std::vector<Animation_Time_Key<glm::quat>> rotationKeys;
	std::vector<Animation_Time_Key<glm::vec3>> positionKeys;
	Node_Animation(const std::string & name = "") : nodeName(name) {}
};
struct Node {
	std::string name;
	glm::mat4 transformation;
	std::vector<Node*> children;
	Node(const std::string & n, const glm::mat4 & t) : name(n), transformation(t) {}
};
struct Animation {
	unsigned int numChannels;
	double ticksPerSecond;
	double duration;
	std::vector<Node_Animation*> channels;
	Animation(const unsigned int & nC = 0, const double & tick = 0, const double & dur = 0)
		: numChannels(nC), ticksPerSecond(tick), duration(dur) {}
};
struct Mesh_Geometry {
	// Per Vertex Attributes
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<glm::vec2> texCoords;
	std::vector<GLuint> materialIndices;

	// Materials
	std::vector<Material> materials;

	// Animation
	std::vector<VertexBoneData> bones;
	std::vector<glm::mat4> boneTransforms;
	std::map<std::string, size_t> boneMap;
	std::vector<Animation> animations;
	Node * rootNode;
};
struct SingleVertex {
	glm::vec3 vertex;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 uv;
	GLuint matID;
	glm::ivec4 boneIDs;
	glm::vec4 weights;
};
struct GeometryInfo {
	std::vector<SingleVertex> m_vertices;
};

/** A static helper class used for reading/writing models.
Uses the Assimp library: http://assimp.sourceforge.net/ */
class Mesh_IO {
public:
	/** Import a model from disk.
	@param	engine			the engine to import to
	@param	relativePath	the path to the file
	@param	importedData	the container to place the imported data within
	@return					true on successfull import, false otherwise (error reported to engine) */
	static bool Import_Model(Engine * engine, const std::string & relativePath, Mesh_Geometry & importedData);
	/** Get the plugin version.
	@return the plugin version */
	static const std::string Get_Version();
};

#endif // Mesh_IO_H