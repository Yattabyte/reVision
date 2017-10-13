/*
	Asset_Texture

	- Encapsulates an OpenGL texture object
	- Support for mipmapping and anistropic filtering
*/

#pragma once
#ifndef	ASSET_TEXTURE
#define	ASSET_TEXTURE
#ifdef	ASSET_TEXTURE_EXPORT
#define	ASSET_TEXTURE_API __declspec(dllexport)
#else
#define	ASSET_TEXTURE_API __declspec(dllimport)
#endif

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

	ASSET_TEXTURE_API ~Asset_Texture();
	ASSET_TEXTURE_API Asset_Texture();
	ASSET_TEXTURE_API Asset_Texture(const string &f, const GLuint &t, const bool &m, const bool &a);
	ASSET_TEXTURE_API static int GetAssetType();
	ASSET_TEXTURE_API virtual void Finalize();

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
	ASSET_TEXTURE_API void Bind(const GLuint &texture_unit);
};


#endif // ASSET_TEXTURE