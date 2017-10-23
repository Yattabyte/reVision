/*
	Asset_Model
	
	- 
*/

#pragma once
#ifndef	ASSET_MODEL
#define	ASSET_MODEL
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define NUM_BONES_PER_VEREX 4
#define NUM_MAX_BONES 100
#include "Managers\Asset_Manager.h"
#include "Assets\Asset_Material.h"
#include "GL\glew.h"
#include "glm\common.hpp"

using namespace glm;

struct VertexBoneData
{
	int IDs[NUM_BONES_PER_VEREX];
	float Weights[NUM_BONES_PER_VEREX];

	VertexBoneData()
	{
		Reset();
	};
	VertexBoneData(const VertexBoneData& vbd) {
		Reset();
		for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
			IDs[i] = vbd.IDs[i];
			Weights[i] = vbd.Weights[i];
		}

	}
	void Reset()
	{
		ZERO_MEM(IDs);
		ZERO_MEM(Weights);
	}
	void AddBoneData(int BoneID, float Weight) {
		for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++)
			if (Weights[i] == 0.0) {
				IDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		assert(0);
	}
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
	std::map<string, int> boneMap;

	AnimationInfo() {};

	// Scene gets destroyed at the end of asset creation
	// We need to copy animation related information
	void SetScene(const aiScene* scene) {
		Animations.resize(scene->mNumAnimations);
		for (int x = 0, total = scene->mNumAnimations; x < total; ++x)
			Animations[x] = new aiAnimation(*scene->mAnimations[x]);

		RootNode = new aiNode(*scene->mRootNode);
	}

	size_t NumAnimations() const { return Animations.size(); }
};

class Asset_Model;
typedef shared_ptr<Asset_Model> Shared_Asset_Model;
class Asset_Model : public Asset
{
public:
	DELTA_CORE_API ~Asset_Model();
	DELTA_CORE_API Asset_Model();
	DELTA_CORE_API Asset_Model(const string & _filename);
	DELTA_CORE_API void Finalize();
	DELTA_CORE_API static int GetAssetType();

	// Model Attributes
	GLuint								gl_vao_ID;
	int									mesh_size;
	string								filename;
	vector<Shared_Asset_Material>		textures;
	GeometryInfo						data;
	AnimationInfo						animationInfo;
	vec3								bbox_min, bbox_max;
};
namespace Asset_Manager {
	DELTA_CORE_API void load_asset(Shared_Asset_Model &user, const string & filename, const bool &threaded = true);
}
#endif // ASSET_MODEL