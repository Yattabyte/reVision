/*
	Asset_Cubemap

	- Encapsulates an OpenGL (cubemap) texture object
*/

#pragma once
#ifndef	ASSET_CUBEMAP
#define	ASSET_CUBEMAP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Assets\Asset.h"
#include "Systems\Asset_Manager.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

class Asset_Cubemap;
typedef shared_ptr<Asset_Cubemap> Shared_Asset_Cubemap;
class Asset_Cubemap : public Asset
{
public:
	/*************
	----Common----
	*************/

	DT_ENGINE_API ~Asset_Cubemap();
	DT_ENGINE_API Asset_Cubemap();
	DT_ENGINE_API Asset_Cubemap(const std::string &f, const GLuint &t);
	DT_ENGINE_API static int GetAssetType();
	DT_ENGINE_API void Finalize();

	/****************
	----Variables----
	****************/

	GLuint			gl_tex_ID, type;
	vec2			size;
	string			filename;
	GLubyte			*pixel_data[6];
	

	/************************
	----Cubemap Functions----
	************************/

	// Makes this texture active at the specific @texture_unit
	DT_ENGINE_API void Bind(const GLuint &texture_unit);
};
namespace Asset_Manager {

};
#endif // ASSET_CUBEMAP