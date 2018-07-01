#pragma once
#ifndef	ASSET_SHADER_PKG_H
#define	ASSET_SHADER_PKG_H
#define EXT_PACKAGE ".pkg"
#define DIRECTORY_SHADER_PKG File_Reader::GetCurrentDir() + "\\Shaders\\"

#include "Assets\Asset.h"
#include "Managers\AssetManager.h"
#include "Utilities\File_Reader.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include <string>

using namespace glm;
using namespace std;
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
	* @param	assetManager	the asset manager to use
	* @param	userAsset		the desired asset container */
	static void CreateDefault(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset);
	/** Begins the creation process for this asset.
	* @param	assetManager	the asset manager to use
	* @param	userAsset		the desired asset container
	* @param	filename		the filename to use
	* @param	threaded		create in a separate thread */
	static void Create(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset, const string & filename, const bool & threaded = true);
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
	static void Initialize(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(AssetManager & assetManager, Shared_Asset_Shader_Pkg & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_PKG_H
