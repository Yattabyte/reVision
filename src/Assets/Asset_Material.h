#pragma once
#ifndef	ASSET_MATERIAL_H
#define	ASSET_MATERIAL_H
#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3

#include "Assets\Asset_Image.h"
#include <vector>
#include "GL\glew.h"
#include "GLM\glm.hpp"


class Engine;
class Asset_Material;
using Shared_Asset_Material = std::shared_ptr<Asset_Material>;

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
	@param	textures		the textures to use 
	@param	threaded		create in a separate thread
	@return					the desired asset */	
	static Shared_Asset_Material Create(Engine * engine, const std::string & filename, const std::vector<std::string> &textures, const bool & threaded = true);
	/** Reading from a .mat file, retrieves the individual file names assigned to this material
	@brief					Updates the appropriate supplied @std::string's with a path to the appropriate file
	@param	filename		the absolute file path of the '.mat' file to read from
	@return					the vector storing texture directories */
	static std::vector<std::string> Get_Material_Textures(const std::string & filename);
	
	
	// Public Attributes
	GLuint m_pboID = 0;
	GLuint m_glArrayID = 0;
	GLuint m_matSpot = 0;
	GLubyte * m_materialData = nullptr;
	glm::ivec2 m_size = glm::ivec2(1);
	std::vector<std::string> m_textures;
	std::vector<Shared_Asset_Image> m_images;


private:
	// Private Constructors
	/** Construct the Material. */
	Asset_Material(const std::string & filename, const std::vector<std::string> & textures);


	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MATERIAL_H