#pragma once
#ifndef	ASSET_CUBEMAP_H
#define	ASSET_CUBEMAP_H

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;
class Engine;
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
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(Engine * engine, Shared_Asset_Cubemap & userAsset);
	/** Begins the creation process for this asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(Engine * engine, Shared_Asset_Cubemap & userAsset, const string & filename, const bool & threaded = true);
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
	static void Initialize(Engine * engine, Shared_Asset_Cubemap & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(Engine * engine, Shared_Asset_Cubemap & userAsset);


	// Private Attributes
	friend class AssetManager;
};
#endif // ASSET_CUBEMAP_H

