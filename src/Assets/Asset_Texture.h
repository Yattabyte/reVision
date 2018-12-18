#pragma once
#ifndef	ASSET_TEXTURE_H
#define	ASSET_TEXTURE_H

#include "Assets\Asset_Image.h"
#include "GL\glad\glad.h"
#include "GLM\glm.hpp"


class Engine;
class Asset_Texture;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Texture : public std::shared_ptr<Asset_Texture> {
public:
	Shared_Texture() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	type			the texture type (2D, 3D, CUBEMAP, etc)
	@param	mipmap			use mipmaps
	@param	anis			use 16x anistropic filtering
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Texture(Engine * engine, const std::string & filename, const GLuint & type = GL_TEXTURE_2D, const bool & mipmap = false, const bool & anis = false, const bool & threaded = true);
};

/** An encapsulation of an OpenGL texture object.
Supports MIP-mapping and anisotropic filtering. */
class Asset_Texture : public Asset
{
public:
	/** Destroy the Texture. */
	~Asset_Texture();
	/** Construct the Texture. */
	Asset_Texture(const std::string & filename);
	/** Construct the Texture with a specific texture type, and optionally enable mipmapping and anisotropic filtering. */
	Asset_Texture(const std::string & filename, const GLuint & t, const bool & m, const bool & a);


	// Public Methods
	/** Makes this texture active at a specific texture unit
	@param	texture_unit	the texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);

	
	// Public Attributes
	GLuint m_glTexID = 0, m_pboID = 0, m_type = GL_TEXTURE_2D;
	bool m_mipmap = false;
	bool m_anis = false;
	Shared_Image m_image;


private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Texture;
};

#endif // ASSET_TEXTURE_H