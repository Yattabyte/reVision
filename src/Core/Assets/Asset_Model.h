/*
	Asset_Model
	
	- An geometric model/mesh
	- To be used in rendering
*/

#pragma once
#ifndef	ASSET_MODEL
#define	ASSET_MODEL
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define NUM_BONES_PER_VEREX 4
#define NUM_MAX_BONES 100
#define GLEW_STATIC

#include "Assets\Asset.h"
#include "Systems\Asset_Manager.h"
#include "Assets\Asset_Material.h"
#include "assimp\scene.h"
#include "GL\glew.h"
#include "glm\common.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>

struct VertexBoneData
{
	int IDs[NUM_BONES_PER_VEREX];
	float Weights[NUM_BONES_PER_VEREX];

	~VertexBoneData();
	VertexBoneData();
	VertexBoneData(const VertexBoneData& vbd);
	void Reset();
	void AddBoneData(int BoneID, float Weight);
};
struct GeometryInfo {
	vector<vec3> vs, nm, tg, bt;
	vector<vec2> uv;
	vector<GLuint> ts;
	vector<VertexBoneData> bones;
};
struct BoneInfo
{
	aiMatrix4x4 BoneOffset;
	aiMatrix4x4 FinalTransformation;
};
struct AnimationInfo {
	vector<aiAnimation*> Animations;
	aiNode *RootNode;
	vector<BoneInfo> meshTransforms;
	map<string, int> boneMap;

	~AnimationInfo();
	AnimationInfo();
	// Scene gets destroyed at the end of asset creation
	// We need to copy animation related information
	void SetScene(const aiScene* scene);
	size_t NumAnimations() const;
};

class Asset_Model;
typedef shared_ptr<Asset_Model> Shared_Asset_Model;
class Asset_Model : public Asset
{
public:
	/*************
	----Common----
	*************/

	DT_ENGINE_API ~Asset_Model();
	DT_ENGINE_API Asset_Model();
	DT_ENGINE_API Asset_Model(const string & _filename);
	DT_ENGINE_API void Finalize();
	DT_ENGINE_API static int GetAssetType();

	/****************
	----Variables----
	****************/

	GLuint								gl_vao_ID;
	int									mesh_size;
	string								filename;
	vector<Shared_Asset_Material>		textures;
	GeometryInfo						data;
	AnimationInfo						animationInfo;
	vec3								bbox_min, bbox_max;
};
namespace Asset_Manager {
	DT_ENGINE_API void load_asset(Shared_Asset_Model &user, const string & filename, const bool &threaded = true);
};
#endif // ASSET_MODEL