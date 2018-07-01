#pragma once
#ifndef	ASSET_MODEL_H
#define	ASSET_MODEL_H
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define GLEW_STATIC
#define EXT_MODEL ".obj"
#define DIRECTORY_MODEL File_Reader::GetCurrentDir() + "\\Models\\"
#define ABS_DIRECTORY_MODEL(filename) DIRECTORY_MODEL + filename + EXT_MODEL
#define DIRECTORY_MODEL_MAT_TEX File_Reader::GetCurrentDir() + "\\Textures\\Environment\\" 

#include "Assets\Asset.h"
#include "Assets\Asset_Material.h"
#include "Managers\ModelManager.h"
#include "Managers\AssetManager.h"
#include "assimp\scene.h"
#include "GL\glew.h"
#include "glm\common.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>

class Asset_Model;
typedef shared_ptr<Asset_Model> Shared_Asset_Model;

struct BoneInfo
{
	mat4 BoneOffset;
	mat4 FinalTransformation;
};

struct AnimationInfo {
	vector<aiAnimation*> Animations;
	aiNode * RootNode;
	vector<BoneInfo> meshTransforms;
	map<string, int> boneMap;

	~AnimationInfo() {}
	AnimationInfo() {}
	// Scene gets destroyed at the end of asset creation
	// We need to copy animation related information
	void setScene(const aiScene * scene);
	size_t numAnimations() const;
};


/**
 * A 3D geometric mesh meant to be used in 3D rendering.
 **/
class Asset_Model : public Asset
{
public:	
	/** Destroy the Model. */
	~Asset_Model();


	// Public Methods
	/** Creates a default asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(AssetManager & assetManager, ModelManager & modelManager, Shared_Asset_Model & userAsset);
	/** Begins the creation process for this asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container
	 * @param	modelManager	the model manager to use
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(AssetManager & assetManager, Shared_Asset_Model & userAsset, ModelManager & modelManager, const string & filename, const bool & threaded = true);
	/** Returns the material ID for a skin given an index into this list
	 * @note			Clamps to the skin list size, so it won't go out of bounds
	 * @param	index	into this model's skin list. 
	 * @return			index into the master material list in which this skin can be found at */	
	GLuint getSkinID(const unsigned int & desired);


	// Public Attributes
	int									m_meshSize;
	vector<Shared_Asset_Material>		m_skins;
	GeometryInfo						m_data;
	AnimationInfo						m_animationInfo;
	vec3								m_bboxMin, m_bboxMax, m_bboxCenter;
	float								m_radius;
	GLint								m_offset, m_count;
	ModelManager						*m_modelManager;


private:
	// Private Constructors
	/** Construct the Model. */
	Asset_Model(const string & filename, ModelManager * modelManager);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(AssetManager & assetManager, ModelManager & modelManager, Shared_Asset_Model & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(AssetManager & assetManager, Shared_Asset_Model & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MODEL_H