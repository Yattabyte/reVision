#pragma once
#ifndef	ASSET_IMAGE_H
#define	ASSET_IMAGE_H

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"


class Engine;
class Asset_Image;
using Shared_Asset_Image = std::shared_ptr<Asset_Image>;

/** Holds image data, and nothing more. */
class Asset_Image : public Asset
{
public:
	/** Destroy the Image. */
	~Asset_Image();


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Image Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	
	
	// Public Attributes
	glm::ivec2 m_size = glm::ivec2(0);
	GLubyte	* m_pixelData = nullptr;
	GLint m_pitch = 0;
	GLuint m_bpp = 0;


private:
	// Private Constructors
	/** Construct the Image. */
	Asset_Image(const std::string & filename);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine) override;
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_IMAGE_H