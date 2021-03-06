#pragma once
#ifndef	TEXTURE_H
#define	TEXTURE_H

#include "Assets/Image.h"


// Forward Declarations
class Texture;

/** Shared version of a Texture asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Texture final : public std::shared_ptr<Texture> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Texture() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use.
	@param	filename		the filename to use.
	@param	type			the texture type (2D, 3D, CUBEMAP, etc).
	@param	mipmap			use mipmaps.
	@param	anisotropy		use 16x anisotropic filtering.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Texture(Engine& engine, const std::string& filename, const GLuint& type = GL_TEXTURE_2D, const bool& mipmap = false, const bool& anisotropy = false, const bool& threaded = true);
};

/** An encapsulation of an OpenGL texture object.
Represents an OpenGL texture object, and is responsible for loading the image from disk.
Supports MIP-mapping and anisotropic filtering.
@note	owns 1 Shared_Image object. */
class Texture final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Texture. */
	~Texture();
	/** Construct the Texture.
	@param	engine			reference to the engine to use.
	@param	filename		the asset file name (relative to engine directory). */
	Texture(Engine& engine, const std::string& filename);
	/** Construct the Texture with a specific texture type, and optionally enable mipmapping and anisotropic filtering.
	@param	engine			reference to the engine to use.
	@param	filename		the asset file name (relative to engine directory).
	@param	type			the texture type (2D, 3D, CUBEMAP, etc).
	@param	mipmap			use mipmaps.
	@param	anisotropy		use 16x anisotropic filtering. */
	Texture(Engine& engine, const std::string& filename, const GLuint& type, const bool& mipmap, const bool& anisotropy);


	// Public Methods
	/** Makes this texture active at a specific texture unit.
	@param	texture_unit	the texture unit to make this texture active at. */
	void bind(const unsigned int& texture_unit) noexcept;


	// Public Attributes
	GLuint m_glTexID = 0, m_type = GL_TEXTURE_2D;
	bool m_mipmap = false;
	bool m_anis = false;
	Shared_Image m_image;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Texture(Texture&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Texture(const Texture&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline Texture& operator =(Texture&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline Texture& operator =(const Texture&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Texture;
};

#endif // TEXTURE_H