#pragma once
#ifndef	ASSET_TEXTURE_H
#define	ASSET_TEXTURE_H

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;
class Engine;
class Asset_Texture;
typedef shared_ptr<Asset_Texture> Shared_Asset_Texture;


/**
 * An encapsulation of an OpenGL texture object.\n
 * Supports MIP-mapping and anisotropic filtering.
 **/
class Asset_Texture : public Asset
{
public:
	/** Destroy the Texture. */
	~Asset_Texture();


	// Public Methods
	/** Creates a default asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(Engine * engine, Shared_Asset_Texture & userAsset);
	/** Begins the creation process for this asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	type			the texture type (2D, 3D, CUBEMAP, etc)
	 * @param	mipmap			use mipmaps
	 * @param	anis			use 16x anistropic filtering
	 * @param	threaded		create in a separate thread */
	static void Create(Engine * engine, Shared_Asset_Texture & userAsset, const string & filename, const GLuint & type = GL_TEXTURE, const bool & mipmap = false, const bool & anis = false, const bool & threaded = true);
	/** Makes this texture active at a specific texture unit
	 * @param	texture_unit	the texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);

	
	// Public Attributes
	GLuint m_glTexID, m_type;
	vec2 m_size;
	GLubyte	* m_pixelData;
	bool m_mipmap;
	bool m_anis;


private:
	// Private Constructors
	/** Construct the Texture. */
	Asset_Texture(const string & filename);
	/** Construct the Texture with a specific texture type, and optionally enable mipmapping and anisotropic filtering. */
	Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a);/** Attempts to create an asset from disk or share one if it already exists */


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(Engine * engine, Shared_Asset_Texture & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(Engine * engine, Shared_Asset_Texture & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_TEXTURE_H