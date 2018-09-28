#pragma once
#ifndef	ASSET_CUBEMAP_H
#define	ASSET_CUBEMAP_H

#include "Assets\Asset_Image.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"


class Engine;
class Asset_Cubemap;
using Shared_Asset_Cubemap = std::shared_ptr<Asset_Cubemap>;

/** Represents an OpenGL cubemap texture object. */
class Asset_Cubemap : public Asset
{
public:
	/** Destroy the Cubemap. */
	~Asset_Cubemap();
	/** Construct the Cubemap. */
	Asset_Cubemap(const std::string & filename);


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Cubemap Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	/** Makes this texture active at a specific texture unit.
	@param	texture_unit	the desired texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);
	
	
	// Public Attributes
	GLuint m_glTexID = 0, m_pboIDs[6] = {0,0,0,0,0,0};
	Shared_Asset_Image m_images[6];


private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};
#endif // ASSET_CUBEMAP_H

