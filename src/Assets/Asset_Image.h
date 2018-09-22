#pragma once
#ifndef	ASSET_IMAGE_H
#define	ASSET_IMAGE_H

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"
#include <any>


class Engine;
class Asset_Image;
using Shared_Asset_Image = std::shared_ptr<Asset_Image>;

/** Holds image data, and nothing more. */
class Asset_Image : public Asset
{
public:
	/** Public Policy Enumerations. */
	const enum Fill_Policy : GLenum {
		Checkered,
		Solid,
	};
	const enum Resize_Policy : GLenum {
		Nearest,
		Linear,
	};


	/** Destroy the Image. */
	~Asset_Image();


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	category		the category of image, if available
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Image Create(Engine * engine, const std::string & filename, const bool & threaded = true, const GLenum & policyFill = Fill_Policy::Checkered, const GLenum & policyResize = Resize_Policy::Linear);
	/** Fill the image with the desired colors, in accordance with the fill policy. 
	@param	primaryColor	the primary color to use
	@param	secondaryColor	the secondary color to use */
	void fill(const glm::uvec4 primaryColor = glm::uvec4(128, 128, 255, 255), const glm::uvec4 secondaryColor = glm::uvec4(0, 0, 0, 255));
	/** Resize the image.
	@param	newSize			the new size to use. */
	void resize(const glm::ivec2 newSize);
	
	// Public Attributes
	glm::ivec2 m_size = glm::ivec2(0);
	GLubyte	* m_pixelData = nullptr;
	GLint m_pitch = 0;
	GLuint m_bpp = 0;
	GLenum m_policyFill = Fill_Policy::Checkered;
	GLenum m_policyResize = Resize_Policy::Linear;


private:
	// Private Constructors
	/** Construct the Image. */
	Asset_Image(const std::string & filename);


	// Private Methods
	// Interface Implementation
	void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_IMAGE_H