/*
	Asset_Shader_Pkg

	- An accessory asset for shaders
	- Stores some functionality for other shaders to include from before compiling
	- Has no functionality on its own
*/

#pragma once
#ifndef	ASSET_SHADER_PKG
#define	ASSET_SHADER_PKG
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_PACKAGE ".pkg"
#define DIRECTORY_SHADER_PKG FileReader::GetCurrentDir() + "\\Shaders\\"

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\FileReader.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include <string>

using namespace glm;
using namespace std;

class Asset_Shader_Pkg;
typedef shared_ptr<Asset_Shader_Pkg> Shared_Asset_Shader_Pkg;
class DT_ENGINE_API Asset_Shader_Pkg : public Asset
{
public:	
	/*************
	----Common----
	*************/

	~Asset_Shader_Pkg();
	Asset_Shader_Pkg(const string & filename);
	static int GetAssetType();
	bool ExistsYet();


	/****************
	----Variables----
	****************/

	string package_text;


	/***********************
	----Shader Functions----
	***********************/

	// Make this shader program active
	string getPackageText() const;
};

namespace Asset_Loader {
	// Attempts to create an asset from disk or share one if it already exists
	DT_ENGINE_API void load_asset(Shared_Asset_Shader_Pkg & user, const string & filename, const bool & threaded = true);
};

class Shader_Pkg_WorkOrder : public Work_Order {
public:
	Shader_Pkg_WorkOrder(Shared_Asset_Shader_Pkg & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Shader_Pkg_WorkOrder() {};
	virtual void Initialize_Order();
	virtual void Finalize_Order();

private:
	bool FetchFileFromDisk(string & returnFile, const string & fileDirectory);
	void Parse();

	/****************
	----Variables----
	****************/

	string m_filename;
	Shared_Asset_Shader_Pkg m_asset;
};

#endif // ASSET_SHADER_PKG
