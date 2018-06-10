#pragma once
#ifndef	ASSET_TEXTURE_H
#define	ASSET_TEXTURE_H
#define EXT_TEXTURE	".png"
#define DIRECTORY_TEXTURE File_Reader::GetCurrentDir() + "\\Textures\\"
#define ABS_DIRECTORY_TEXTURE(filename) DIRECTORY_TEXTURE + filename + EXT_TEXTURE

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;
class Asset_Texture;
typedef shared_ptr<Asset_Texture> Shared_Asset_Texture;


/**
 * An encapsulation of an OpenGL texture object.\n
 * Supports MIP-mapping and anisotropic filtering.
 **/
class Asset_Texture : public Asset
{
public:
	// (de)Constructors
	/** Destroy the Texture. */
	~Asset_Texture();
	/** Construct the Texture. */
	Asset_Texture(const string & filename);
	/** Construct the Texture with a specific texture type, and optionally enable mipmapping and anisotropic filtering. */
	Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a);


	// Public Methods
	/** Makes this texture active at a specific texture unit
	 * @param	texture_unit	the texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);

	
	// Public Attributes
	GLuint m_glTexID, m_type;
	vec2 m_size;
	GLubyte	* m_pixelData;
	bool m_mipmap;
	bool m_anis;
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	 void load_asset(Shared_Asset_Texture & user, const string & filename, const bool & mipmap = false, const bool & anis = false, const bool & threaded = true);
};

/**
 * Implements a work order for Texture Assets.
 **/
class Texture_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Texture work order */
	Texture_WorkOrder(Shared_Asset_Texture & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Texture_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Attributes
	string m_filename;
	Shared_Asset_Texture m_asset;
};

#endif // ASSET_TEXTURE_H