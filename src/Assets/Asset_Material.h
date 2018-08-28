#pragma once
#ifndef	ASSET_MATERIAL_H
#define	ASSET_MATERIAL_H
#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"


class Engine;
class Asset_Material;
typedef std::shared_ptr<Asset_Material> Shared_Asset_Material;

/** A collection of texture surfaces that are used together to approximate a real surface.
- Support for:
	- Albedo
	- normal
	- metalness
	- roughness
	- height (WIP)
	- occlusion (in conjunction with SSAO)
- Supports omission of any or all of the files
- Expects all textures in a material to be the same dimension, and will forcefully resize them (in memory). */
class Asset_Material : public Asset
{
public:
	/** Destroy the Material. */
	~Asset_Material();
	

	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use 
	@param	threaded		create in a separate thread
	@return					the desired asset */	
	static Shared_Asset_Material Create(Engine * engine, const std::string & material_filename, const bool & threaded = true);	
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	textures		the textures to use 
	@param	threaded		create in a separate thread
	@return					the desired asset */	
	static Shared_Asset_Material Create(Engine * engine, const std::string(&textures)[MAX_PHYSICAL_IMAGES], const bool & threaded = true);
	/** Reading from a .mat file, retrieves the individual file names assigned to this material
	@brief					Updates the appropriate supplied @std::string's with a path to the appropriate file
	@param	filename		the absolute file path of the '.mat' file to read from
	@param	albedo			reference updated with albedo texture file path
	@param	normal			reference updated with normal texture file path
	@param	metalness		reference updated with metalness texture file path
	@param	roughness		reference updated with roughness texture file path
	@param	height			reference updated with height texture file path
	@param	occlusion		reference updated with occlusion texture file path */
	static void Get_PBR_Properties(const std::string & filename, std::string & albedo = std::string(), std::string & normal = std::string(), std::string & metalness = std::string(), std::string & roughness = std::string(), std::string & height = std::string(), std::string & occlusion = std::string());
	
	
	// Public Attributes
	GLuint m_glArrayID;
	GLuint m_matSpot;
	GLubyte * m_materialData;
	glm::ivec2 m_size;
	std::string m_textures[MAX_PHYSICAL_IMAGES];


private:
	// Private Constructors
	/** Construct the Material. */
	Asset_Material(const std::string & filename);
	/** Construct the Material with a manual set of textures. */
	Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES]);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & fullDirectory);
	virtual void finalize(Engine * engine);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MATERIAL_H