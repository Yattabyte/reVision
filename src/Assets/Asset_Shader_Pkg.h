#pragma once
#ifndef	ASSET_SHADER_PKG_H
#define	ASSET_SHADER_PKG_H

#include "Assets\Asset.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include <string>

using namespace glm;
using namespace std;
class Engine;
class Asset_Shader_Pkg;
typedef shared_ptr<Asset_Shader_Pkg> Shared_Asset_Shader_Pkg;


/**
 * An accessory asset for shaders that stores code blocks for other shaders to use.
 * @brief	no functionality on its own, but can recursively import more code blocks for other shaders and itself.
 **/
class Asset_Shader_Pkg : public Asset
{
public:
	/** Destroy the Shader Package. */
	~Asset_Shader_Pkg();	

	
	// Public Methods
	/** Creates a default asset.
	 * @param	engine			the engine being used
	* @param	userAsset		the desired asset container */
	static void CreateDefault(Engine * engine, Shared_Asset_Shader_Pkg & userAsset);
	/** Begins the creation process for this asset.
	 * @param	engine			the engine being used
	* @param	userAsset		the desired asset container
	* @param	filename		the filename to use
	* @param	threaded		create in a separate thread */
	static void Create(Engine * engine, Shared_Asset_Shader_Pkg & userAsset, const string & filename, const bool & threaded = true);
	/** Retrieves this package's content as a string.
	 * @return	package contents */
	string getPackageText() const;

	
	// Public Attributes
	string m_packageText;


private:
	// Private Constructors
	/** Construct the Shader Package. */
	Asset_Shader_Pkg(const string & filename);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(Engine * engine, Shared_Asset_Shader_Pkg & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(Engine * engine, Shared_Asset_Shader_Pkg & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_PKG_H
