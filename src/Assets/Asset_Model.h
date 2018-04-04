#pragma once
#ifndef	ASSET_MODEL
#define	ASSET_MODEL
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define GLEW_STATIC
#define EXT_MODEL ".obj"
#define DIRECTORY_MODEL File_Reader::GetCurrentDir() + "\\Models\\"
#define ABS_DIRECTORY_MODEL(filename) DIRECTORY_MODEL + filename + EXT_MODEL
#define DIRECTORY_MODEL_MAT_TEX File_Reader::GetCurrentDir() + "\\Textures\\Environment\\" 

#include "Assets\Asset.h"
#include "Assets\Asset_Material.h"
#include "Managers\Asset_Manager.h"
#include "assimp\scene.h"
#include "GL\glew.h"
#include "glm\common.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>


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

	~AnimationInfo();
	AnimationInfo();
	// Scene gets destroyed at the end of asset creation
	// We need to copy animation related information
	void setScene(const aiScene * scene);
	size_t numAnimations() const;
};
class Asset_Model;
typedef shared_ptr<Asset_Model> Shared_Asset_Model;


/**
 * A 3D geometric mesh meant to be used in 3D rendering.
 **/
class DT_ENGINE_API Asset_Model : public Asset
{
public:	
	// (de)Constructors
	/** Destroy the Model. */
	~Asset_Model();
	/** Construct the Model. */
	Asset_Model(const string & filename);


	// Interface Implementations
	/** Returns whether or not this asset has completed finalizing.
	* @return			true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();


	// Public Methods
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
	vec3								m_bboxMin, m_bboxMax;
	GLint								m_offset, m_count;
	GLsync								m_fence;
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	DT_ENGINE_API void load_asset(Shared_Asset_Model & user, const string & filename, const bool & threaded = true);
};

/**
 * Implements a work order for Model Assets.
 **/
class Model_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Model work order */
	Model_WorkOrder(Shared_Asset_Model & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Model_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Methods
	/** Generates and parses the bones for our model.
	 * @param	model	the model to load the bones onto
	 * @param	scene	the model scene to load the bones from **/
	void initializeBones(Shared_Asset_Model & model, const aiScene * scene);	
	/** Generates and parses the materials for our model from the model scene.
	 * @param	modelMaterial	the material asset to load the model material into
	 * @param	sceneMaterial	the model scene material to parse from **/
	void generateMaterial(Shared_Asset_Material & modelMaterial, const aiMaterial * sceneMaterial);
	/** Generates and parses the materials for our model as a last resort.
	 * @brief	attempts to load a material from disk as it doesn't have any aiMaterial's to load from.
	 * @param	modelMaterial	the material asset to load the model material into */
	void generateMaterial(Shared_Asset_Material & modelMaterial);


	// Private Attributes
	string m_filename;
	Shared_Asset_Model m_asset;
};
#endif // ASSET_MODEL