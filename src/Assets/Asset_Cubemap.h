#pragma once
#ifndef	ASSET_CUBEMAP_H
#define	ASSET_CUBEMAP_H
#define EXT_CUBEMAP ".png"
#define DIRECTORY_CUBEMAP File_Reader::GetCurrentDir() + "\\Textures\\Cubemaps\\"
#define ABS_DIRECTORY_CUBEMAP(filename) DIRECTORY_CUBEMAP + filename

#include "Assets\Asset.h"
#include "Managers\AssetManager.h"
#include "Utilities\File_Reader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;
class Asset_Cubemap;
typedef shared_ptr<Asset_Cubemap> Shared_Asset_Cubemap;


/**
 * Represents an OpenGL cubemap texture object.
 **/
class Asset_Cubemap : public Asset
{
public:
	/** Destroy the Cubemap. */
	~Asset_Cubemap();


	// Public Methods
	/** Creates a default asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(AssetManager & assetManager, Shared_Asset_Cubemap & userAsset);
	/** Begins the creation process for this asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(AssetManager & assetManager, Shared_Asset_Cubemap & userAsset, const string & filename, const bool & threaded = true);
	/** Makes this texture active at a specific texture unit.
	 * @param	texture_unit	the desired texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);

	
	// Public Attributes
	GLuint m_glTexID;
	vec2 m_size;
	GLubyte	* m_pixelData[6];


private:
	// Private Constructors
	/** Construct the Cubemap. */
	Asset_Cubemap(const std::string & filename);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(AssetManager & assetManager, Shared_Asset_Cubemap & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(AssetManager & assetManager, Shared_Asset_Cubemap & userAsset);


	// Private Attributes
	friend class AssetManager;
};
#endif // ASSET_CUBEMAP_H

