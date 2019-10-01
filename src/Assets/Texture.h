#pragma once
#ifndef	TEXTURE_H
#define	TEXTURE_H

#include "Assets/Image.h"
#include "glm/glm.hpp"


class Engine;
class Texture;

/** Shared version of a Texture asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Texture final : public std::shared_ptr<Texture> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Texture() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	type			the texture type (2D, 3D, CUBEMAP, etc).
	@param	mipmap			use mipmaps.
	@param	anis			use 16x anistropic filtering.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Texture(Engine* engine, const std::string& filename, const GLuint& type = GL_TEXTURE_2D, const bool& mipmap = false, const bool& anis = false, const bool& threaded = true);
};

/** An encapsulation of an OpenGL texture object.
Represents an OpenGL texture object, and is responsible for loading the image from disk.
Supports MIP-mapping and anisotropic filtering.
@note	owns 1 Shared_Image object. */
class Texture final : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Texture. */
	~Texture();
	/** Construct the Texture.
	@param	engine			the engine to use.
	@param	filename		the asset file name (relative to engine directory). */
	Texture(Engine* engine, const std::string& filename);
	/** Construct the Texture with a specific texture type, and optionally enable mipmapping and anisotropic filtering.
	@param	engine			the engine to use.
	@param	filename		the asset file name (relative to engine directory).
	@param	type			the texture type (2D, 3D, CUBEMAP, etc).
	@param	mipmap			use mipmaps.
	@param	anis			use 16x anistropic filtering. */
	Texture(Engine* engine, const std::string& filename, const GLuint& type, const bool& mipmap, const bool& anis);


	// Public Methods
	/** Makes this texture active at a specific texture unit.
	@param	texture_unit	the texture unit to make this texture active at. */
	void bind(const unsigned int& texture_unit);


	// Public Attributes
	GLuint m_glTexID = 0, m_pboID = 0, m_type = GL_TEXTURE_2D;
	bool m_mipmap = false;
	bool m_anis = false;
	Shared_Image m_image;


private:
	// Private Interface Implementation
	virtual void initialize() override final;


	// Private Attributes
	friend class Shared_Texture;
};

#endif // TEXTURE_H