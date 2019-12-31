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

/** Container mapping vertices to skeleton bones. */
struct VertexBoneData {
	int IDs[NUM_BONES_PER_VEREX]{};
	float Weights[NUM_BONES_PER_VEREX]{};
	/** Reset the bone ID's and weights to zero. */
	void Reset();
	/** Add a bone to this vertex.
	@param	BoneID	the ID of the bone in the skeleton.
	@param	Weight	the weight of the bone. */
	void AddBoneData(const int& BoneID, const float& Weight) noexcept;
};
/** Container for materials' component textures. */
struct Material_Strings {
	std::string albedo, normal, metalness, roughness, height, ao;
};

/** Container for model animation key frames.
@tparam	T		the data type for the key frame (vec3, quat, etc). */
template<typename T = glm::vec3>
struct Animation_Time_Key {
	double time = 0.0;
	T value = T();
};
/** Container for a model's animation. */
struct Node_Animation {
	std::string nodeName;
	std::vector<Animation_Time_Key<glm::vec3>> scalingKeys;
	std::vector<Animation_Time_Key<glm::quat>> rotationKeys;
	std::vector<Animation_Time_Key<glm::vec3>> positionKeys;
};
/** Container for a model's node. */
struct Node {
	std::string name;
	glm::mat4 transformation = glm::mat4(1);
	std::vector<Node> children;
};
/** Container for animation. */
struct Animation {
	unsigned int numChannels = 0;
	double ticksPerSecond = 0.0;
	double duration = 0.0;
	std::vector<Node_Animation> channels;
};
/** Container for underlying mesh data. */
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
/** Container defining a single vertex. */
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
/** Container defining a collection of vertices. */
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