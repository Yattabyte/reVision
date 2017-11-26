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
#define DIRECTORY_TEXTURE getCurrentDir() + "\\Textures\\"
#define ABS_DIRECTORY_TEXTURE(filename) DIRECTORY_TEXTURE + filename + EXT_TEXTURE

#include "Assets\Asset.h"
#include "Systems\Asset_Manager.h"
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

	DT_ENGINE_API ~Asset_Texture();
	DT_ENGINE_API Asset_Texture();
	DT_ENGINE_API Asset_Texture(const string &f, const GLuint &t, const bool &m, const bool &a);
	DT_ENGINE_API static int GetAssetType();
	DT_ENGINE_API virtual void Finalize();

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
	DT_ENGINE_API void Bind(const GLuint &texture_unit);
};
namespace Asset_Manager {
	DT_ENGINE_API void load_asset(Shared_Asset_Texture &user, const string & filename, const bool &mipmap = false, const bool &anis = false, const bool &threaded = true);
};
#endif // ASSET_TEXTURE