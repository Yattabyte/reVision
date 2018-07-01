#pragma once
#ifndef	ASSET_MATERIAL_H
#define	ASSET_MATERIAL_H
#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3
#define EXT_MATERIAL ".mat"
#define ABS_DIRECTORY_MATERIAL(filename) File_Reader::GetCurrentDir() + "\\Materials\\" + filename + EXT_MATERIAL
#define ABS_DIRECTORY_MAT_TEX(filename) File_Reader::GetCurrentDir() + "\\Textures\\Environment\\" + filename

#include "Assets\Asset.h"
#include "Managers\AssetManager.h"
#include "Utilities\File_Reader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;
class Asset_Material;
typedef shared_ptr<Asset_Material> Shared_Asset_Material;


/**
 * A collection of texture surfaces that are used together to approximate a real surface.
 * - Support for:
 *		- Albedo
 *		- normal
 *		- metalness
 *		- roughness
 *		- height (WIP)
 *		- occlusion (in conjunction with SSAO)
 * - Supports omission of any or all of the files
 * - Expects all textures in a material to be the same dimension, and will forcefully resize them (in memory)
 **/
class Asset_Material : public Asset
{
public:
	/** Destroy the Material. */
	~Asset_Material();
	

	// Public Methods
	/** Creates a default asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(AssetManager & assetManager, Shared_Asset_Material & userAsset);
	/** Begins the creation process for this asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use 
	 * @param	threaded		create in a separate thread */
	static void Create(AssetManager & assetManager, Shared_Asset_Material & userAsset, const std::string & material_filename, const bool & threaded, const std::string(&textures)[MAX_PHYSICAL_IMAGES]);
	/** Reading from a .mat file, retrieves the individual file names assigned to this material
	 * @brief				Updates the appropriate supplied @string's with a path to the appropriate file
	 * @param	filename	the absolute file path of the '.mat' file to read from
	 * @param	albedo		reference updated with albedo texture file path
	 * @param	normal		reference updated with normal texture file path
	 * @param	metalness	reference updated with metalness texture file path
	 * @param	roughness	reference updated with roughness texture file path
	 * @param	height		reference updated with height texture file path
	 * @param	occlusion	reference updated with occlusion texture file path */
	static void Get_PBR_Properties(const string & filename, string & albedo = string(), string & normal = string(), string & metalness = string(), string & roughness = string(), string & height = string(), string & occlusion = string());
	
	
	// Public Attributes
	GLuint m_glArrayID;
	GLuint m_matSpot;
	GLubyte * m_materialData;
	vec2 m_size;
	string m_textures[MAX_PHYSICAL_IMAGES];


private:
	// Private Constructors
	/** Construct the Material. */
	Asset_Material(const std::string & filename);
	/** Construct the Material with a specific index.*/
	Asset_Material(const std::string & filename, const GLuint & spot);
	/** Construct the Material with a manual set of textures, and a specific index. */
	Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES], const GLuint & spot);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(AssetManager & assetManager, Shared_Asset_Material & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(AssetManager & assetManager, Shared_Asset_Material & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MATERIAL_H