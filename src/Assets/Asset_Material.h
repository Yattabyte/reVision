#pragma once
#ifndef	ASSET_MATERIAL
#define	ASSET_MATERIAL
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3
#define EXT_MATERIAL ".mat"
#define ABS_DIRECTORY_MATERIAL(filename) File_Reader::GetCurrentDir() + "\\Materials\\" + filename + EXT_MATERIAL
#define ABS_DIRECTORY_MAT_TEX(filename) File_Reader::GetCurrentDir() + "\\Textures\\Environment\\" + filename

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
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
class DT_ENGINE_API Asset_Material : public Asset
{
public:
	// (de)Constructors
	/** Destroy the Material. */
	~Asset_Material();
	/** Construct the Material. */
	Asset_Material(const std::string & filename);
	/** Construct the Material with a specific index.*/
	Asset_Material(const std::string & filename, const GLuint & spot);
	/** Construct the Material with a manual set of textures, and a specific index. */
	Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES], const GLuint & spot);


	// Interface Implementations
	/** Returns whether or not this asset has completed finalizing.
	* @return				true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();


	// Public Methods
	/** @todo delete */
	static int Get_Asset_Type();
	/** Apply a specific set of textures to be used as a material.
	 * @param	tx	an array of MAX_PHYSICAL_IMAGES length, formatted as a list of material textures (albedo/normal/metalness/roughness/height/occlusion) */
	void setTextures(const std::string(&tx)[MAX_PHYSICAL_IMAGES]);
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
	string textures[MAX_PHYSICAL_IMAGES];
	GLuint gl_array_ID;
	vec2 size;
	GLubyte * materialData;
	GLuint mat_spot;
	GLsync m_fence;	
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	DT_ENGINE_API void load_asset(Shared_Asset_Material & user, const std::string(&textures)[MAX_PHYSICAL_IMAGES], const bool & threaded = true);
	
	/** Attempts to create an asset from disk or share one if it already exists */
	DT_ENGINE_API void load_asset(Shared_Asset_Material & user, const std::string & material_filename, const bool & threaded = true);
};

/**
 * Implements a work order for Material Assets.
 **/
class Material_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Material work order */
	Material_WorkOrder(Shared_Asset_Material & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Material_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Attributes
	string m_filename;
	Shared_Asset_Material m_asset;	
};

#endif // ASSET_MATERIAL