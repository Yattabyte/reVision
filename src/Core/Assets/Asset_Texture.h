/*
	Asset_Texture

	- Encapsulates an OpenGL texture object
	- Support for mipmapping and anistropic filtering
*/

#pragma once
#ifndef	ASSET_TEXTURE
#define	ASSET_TEXTURE
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define EXT_TEXTURE	".png"
#define DIRECTORY_TEXTURE getCurrentDir() + "\\Textures\\"
#define ABS_DIRECTORY_TEXTURE(filename) DIRECTORY_TEXTURE + filename + EXT_TEXTURE

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

class Asset_Texture;
typedef shared_ptr<Asset_Texture> Shared_Asset_Texture;
class Asset_Texture : public Asset
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Asset_Texture();
	DELTA_CORE_API Asset_Texture();
	DELTA_CORE_API Asset_Texture(const string &f, const GLuint &t, const bool &m, const bool &a);
	DELTA_CORE_API static int GetAssetType();
	DELTA_CORE_API virtual void Finalize();

	/****************
	----Variables----
	****************/

	GLuint gl_tex_ID, type;
	vec2 size;
	string filename;
	GLubyte	*pixel_data;
	bool mipmap;
	bool anis;

	/************************
	----Texture Functions----
	************************/

	// Makes this texture active at the specific @texture_unit
	DELTA_CORE_API void Bind(const GLuint &texture_unit);
};
namespace Asset_Manager {
	DELTA_CORE_API void load_asset(Shared_Asset_Texture &user, const string & filename, const bool &mipmap = false, const bool &anis = false, const bool &threaded = true);
};
#endif // ASSET_TEXTURE