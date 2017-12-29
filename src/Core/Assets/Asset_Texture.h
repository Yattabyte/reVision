/*
	Asset_Texture

	- Encapsulates an OpenGL texture object
	- Support for mipmapping and anistropic filtering
*/

#pragma once
#ifndef	ASSET_TEXTURE
#define	ASSET_TEXTURE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_TEXTURE	".png"
#define DIRECTORY_TEXTURE FileReader::GetCurrentDir() + "\\Textures\\"
#define ABS_DIRECTORY_TEXTURE(filename) DIRECTORY_TEXTURE + filename + EXT_TEXTURE

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\FileReader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

class Asset_Texture;
typedef shared_ptr<Asset_Texture> Shared_Asset_Texture;
class DT_ENGINE_API Asset_Texture : public Asset
{
public:
	/*************
	----Common----
	*************/

	~Asset_Texture();
	Asset_Texture(const string & filename);
	Asset_Texture(const string & filename, const GLuint & t, const bool & m, const bool & a);
	static int GetAssetType();
	bool ExistsYet();


	/****************
	----Variables----
	****************/

	GLuint gl_tex_ID, type;
	vec2 size;
	GLubyte	* pixel_data;
	GLsync m_fence;
	bool mipmap;
	bool anis;


	/************************
	----Texture Functions----
	************************/

	// Makes this texture active at the specific @texture_unit
	void Bind(const GLuint &texture_unit);
};

namespace Asset_Loader {
	DT_ENGINE_API void load_asset(Shared_Asset_Texture & user, const string & filename, const bool & mipmap = false, const bool & anis = false, const bool & threaded = true);
};

class Texture_WorkOrder : public Work_Order {
public:
	Texture_WorkOrder(Shared_Asset_Texture & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Texture_WorkOrder() {};
	virtual void Initialize_Order();
	virtual void Finalize_Order();

private:
	string m_filename;
	Shared_Asset_Texture m_asset;
};

#endif // ASSET_TEXTURE