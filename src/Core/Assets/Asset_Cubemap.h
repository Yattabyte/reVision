/*
	Asset_Cubemap

	- Encapsulates an OpenGL cubemap texture object
*/

#pragma once
#ifndef	ASSET_CUBEMAP
#define	ASSET_CUBEMAP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_CUBEMAP ".png"
#define DIRECTORY_CUBEMAP FileReader::GetCurrentDir() + "\\Textures\\Cubemaps\\"
#define ABS_DIRECTORY_CUBEMAP(filename) DIRECTORY_CUBEMAP + filename

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\FileReader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

class Asset_Cubemap;
typedef shared_ptr<Asset_Cubemap> Shared_Asset_Cubemap;
class DT_ENGINE_API Asset_Cubemap : public Asset
{
public:
	/*************
	----Common----
	*************/

	~Asset_Cubemap();
	Asset_Cubemap();
	Asset_Cubemap(const std::string &f);
	static int GetAssetType();


	/****************
	----Variables----
	****************/

	GLuint			gl_tex_ID;
	vec2			size;
	string			filename;
	GLubyte			*pixel_data[6];
	

	/************************
	----Cubemap Functions----
	************************/

	// Makes this texture active at the specific @texture_unit
	void Bind(const GLuint &texture_unit);
};

namespace Asset_Manager {
	DT_ENGINE_API void load_asset(Shared_Asset_Cubemap & user, const string & filename, const bool &threaded = true);
};

class Cubemap_WorkOrder : public Work_Order {
public:
	Cubemap_WorkOrder(Shared_Asset_Cubemap &asset, const std::string &filename) : m_asset(asset), m_filename(filename) {};
	~Cubemap_WorkOrder() {};
	virtual void Initialize_Order();
	virtual void Finalize_Order();

private:
	std::string m_filename;
	Shared_Asset_Cubemap m_asset;
};
#endif // ASSET_CUBEMAP

