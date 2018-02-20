#pragma once
#ifndef	ASSET_SHADER_PKG
#define	ASSET_SHADER_PKG
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_PACKAGE ".pkg"
#define DIRECTORY_SHADER_PKG File_Reader::GetCurrentDir() + "\\Shaders\\"

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
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
class DT_ENGINE_API Asset_Shader_Pkg : public Asset
{
public:
	// (de)Constructors
	/** Destroy the Shader Package. */
	~Asset_Shader_Pkg();	
	/** Construct the Shader Package. */
	Asset_Shader_Pkg(const string & filename);


	// Interface Implementations
	/** Returns whether or not this asset has completed finalizing.
	* @return	true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();


	// Public Methods
	/** Retrieves this package's content as a string.
	 * @return	package contents */
	string getPackageText() const;

	
	// Public Attributes
	string package_text;
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	DT_ENGINE_API void load_asset(Shared_Asset_Shader_Pkg & user, const string & filename, const bool & threaded = true);
};

/**
 * Implements a work order for Shader Package Assets.
 **/
class Shader_Pkg_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Shader_Pkg work order */
	Shader_Pkg_WorkOrder(Shared_Asset_Shader_Pkg & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Shader_Pkg_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Methods
	/** Reads in a text file from disk.
	 * @param	returnFile	reference string to return the text file to
	 * @param	fileDirectory	absolute path to the file to read from
	 * @return	true if the file exists, false otherwise */
	bool fetchFileFromDisk(string & returnFile, const string & fileDirectory);

	/** Parses the document for further inclusions and imports when necessary. */
	void parse();


	// Private Attributes
	string m_filename;
	Shared_Asset_Shader_Pkg m_asset;
};

#endif // ASSET_SHADER_PKG
