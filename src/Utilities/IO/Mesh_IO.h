#pragma once
#ifndef	MESH_IO_H
#define	MESH_IO_H
#define NUM_BONES_PER_VEREX 4

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <map>
#include <string>
#include <vector>


// Forward Declarations
class Engine;

struct VertexBoneData {
	int IDs[NUM_BONES_PER_VEREX]{};
	float Weights[NUM_BONES_PER_VEREX]{};
	void Reset() noexcept;
	void AddBoneData(const int& BoneID, const float& Weight) noexcept;
};
struct Material_Strings {
	std::string albedo = "albedo", normal = "normal", metalness = "metalness", roughness = "roughness", height = "height", ao = "ao";
	inline Material_Strings(const std::string& al, const std::string& n, const std::string& m, const std::string& r, const std::string& h, const std::string& a)
		: albedo(al), normal(n), metalness(m), roughness(r), height(h), ao(a) {}
};
template<typename T>
struct Animation_Time_Key {
	double time = 0.0;
	T value = T();
	inline Animation_Time_Key() noexcept {};
	inline Animation_Time_Key(const double& t, const T& v) noexcept : time(t), value(v) {};
};
struct Node_Animation {
	std::string nodeName = "";
	std::vector<Animation_Time_Key<glm::vec3>> scalingKeys;
	std::vector<Animation_Time_Key<glm::quat>> rotationKeys;
	std::vector<Animation_Time_Key<glm::vec3>> positionKeys;
	inline Node_Animation() {};
	inline Node_Animation(const std::string& name) : nodeName(name) {}
};
struct Node {
	std::string name = "";
	glm::mat4 transformation = glm::mat4(1);
	std::vector<Node> children;
	inline Node() {};
	inline Node(const std::string& n, const glm::mat4& t) : name(n), transformation(t) {}
};
struct Animation {
	unsigned int numChannels = 0;
	double ticksPerSecond = 0.0;
	double duration = 0.0;
	std::vector<Node_Animation> channels;
	inline Animation(const unsigned int& nC = 0, const double& tick = 0, const double& dur = 0) noexcept
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
	Node rootNode;
};
struct SingleVertex {
	glm::vec3 vertex = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec3 tangent = glm::vec3(0.0f);
	glm::vec3 bitangent = glm::vec3(0.0f);
	glm::vec2 uv = glm::vec2(0.0f);
	GLuint matID = 0;
	glm::ivec4 boneIDs = glm::ivec4(0);
	glm::vec4 weights = glm::vec4(0.0f);
};
struct GeometryInfo {
	std::vector<SingleVertex> m_vertices;
};

/** A static helper class used for reading/writing models.
Uses the Assimp library: http://assimp.sourceforge.net/ */
class Mesh_IO {
public:
	/** Import a model from disk.
	@param	engine			reference to the engine to use. 
	@param	relativePath	the path to the file.
	@param	importedData	reference to the container to place the imported data within.
	@return					true on successful import, false otherwise (error reported to engine). */
	static bool Import_Model(Engine& engine, const std::string& relativePath, Mesh_Geometry& importedData);
	/** Retrieve the plugin version.
	@return					the plugin version. */
	static std::string Get_Version();
	/** Retrieve a list of all supported model extensions.
	@return					list of supported file types. */
	static std::vector<std::string> Get_Supported_Types();
};

#endif // MESH_IO_H