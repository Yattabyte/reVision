#pragma once
#ifndef	Mesh_IO_H
#define	Mesh_IO_H
#define NUM_BONES_PER_VEREX 4

#include "Utilities/GL/glad/glad.h"
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
	inline ~VertexBoneData() = default;
	VertexBoneData();
	VertexBoneData(const VertexBoneData& vbd);
	void Reset();
	void AddBoneData(const int& BoneID, const float& Weight);
};
struct Material_Strings {
	std::string albedo = "", normal = "", metalness = "", roughness = "", height = "", ao = "";
	inline Material_Strings(const std::string& al = "albedo", const std::string& n = "normal", const std::string& m = "metalness", const std::string& r = "roughness", const std::string& h = "height", const std::string& a = "ao")
		: albedo(al), normal(n), metalness(m), roughness(r), height(h), ao(a) {}
};
template<typename T>
struct Animation_Time_Key {
	double time = 0.0;
	T value = T();
	inline Animation_Time_Key() {};
	inline Animation_Time_Key(const double& t, const T& v) : time(t), value(v) {};
};
struct Node_Animation {
	std::string nodeName = "";
	std::vector<Animation_Time_Key<glm::vec3>> scalingKeys;
	std::vector<Animation_Time_Key<glm::quat>> rotationKeys;
	std::vector<Animation_Time_Key<glm::vec3>> positionKeys;
	inline Node_Animation(const std::string& name = "") : nodeName(name) {}
};
struct Node {
	std::string name = "";
	glm::mat4 transformation = glm::mat4(1);
	std::vector<Node*> children;
	inline Node(const std::string& n, const glm::mat4& t) : name(n), transformation(t) {}
};
struct Animation {
	unsigned int numChannels = 0;
	double ticksPerSecond = 0.0;
	double duration = 0.0;
	std::vector<Node_Animation*> channels;
	inline Animation(const unsigned int& nC = 0, const double& tick = 0, const double& dur = 0)
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
	std::vector<GLuint> meshIndices;

	// Materials
	std::vector<Material_Strings> materials;

	// Animation
	std::vector<VertexBoneData> bones;
	std::vector<glm::mat4> boneTransforms;
	std::map<std::string, size_t> boneMap;
	std::vector<Animation> animations;
	Node* rootNode;
};
struct SingleVertex {
	glm::vec3 vertex;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 uv;
	GLuint matID = 0;
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
	static bool Import_Model(Engine* engine, const std::string& relativePath, Mesh_Geometry& importedData);
	/** Get the plugin version.
	@return					the plugin version */
	static std::string Get_Version();
};

#endif // Mesh_IO_H