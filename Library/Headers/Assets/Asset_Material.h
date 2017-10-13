/*
	Asset_Material

	- A material object is a collection of textures that are used together to describe a surface
	- Support for: 
		- Albedo
		- normal
		- metalness
		- roughness
		- height
		- occlusion
	- Any one of these can be omitted safely!
	- Images used in a material SHOULD ALWAYS be the same size!
	- ALL missing data is auto generated using fallback values. Used when image file missing OR an image is smaller than the rest in set (missing region filled in)
	- height map support is VERY WIP and doesn't look that good yet
	- occlusion maps get rendered OVER by SSAO, blending together, allowing authored indirect lighting occlusion
*/

#pragma once
#ifndef	ASSET_MATERIAL
#define	ASSET_MATERIAL
#ifdef	ASSET_MATERIAL_EXPORT
#define	ASSET_MATERIAL_API __declspec(dllexport)
#else
#define	ASSET_MATERIAL_API __declspec(dllimport)
#endif

#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3

#include "Managers\Asset_Manager.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

class Asset_Material;
typedef shared_ptr<Asset_Material> Shared_Asset_Material;
class Asset_Material : public Asset
{
public:
	/*************
	----Common----
	*************/

	ASSET_MATERIAL_API ~Asset_Material();
	ASSET_MATERIAL_API Asset_Material();
	ASSET_MATERIAL_API Asset_Material(const std::string &_file, const GLuint & mat_buf_id, const GLuint & spot);
	ASSET_MATERIAL_API Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES], const GLuint & mat_buf_id, const GLuint & spot);
	ASSET_MATERIAL_API static int GetAssetType();
	ASSET_MATERIAL_API void Finalize();

	/****************
	----Variables----
	****************/
	std::string material_filename;
	std::string textures[MAX_PHYSICAL_IMAGES];
	GLuint gl_array_ID;
	GLuint64 handle;
	vec2 size;
	GLubyte *materialData;
	GLuint matbuffer_ID;
	GLuint mat_spot;


	/*************************
	----Material Functions----
	*************************/

	// Material textures are technically separate files, so we used a custom .mat file to direct which files to use for what texture spot
	// Updates the appropriate supplied @string's with a path to the appropriate file
	ASSET_MATERIAL_API static void getPBRProperties(const string & filename, string & albedo = string(), string & normal = string(), string & metalness = string(), string & roughness = string(), string & height = string(), string & occlusion = string());
};
#endif // ASSET_MATERIAL