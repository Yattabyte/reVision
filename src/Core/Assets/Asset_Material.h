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
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define MAX_PHYSICAL_IMAGES 6
#define MAX_DIGITAL_IMAGES 3
#define EXT_MATERIAL ".mat"
#define DIRECTORY_MATERIAL getCurrentDir() + "\\Materials\\"
#define ABS_DIRECTORY_MATERIAL(filename) DIRECTORY_MATERIAL + filename + EXT_MATERIAL

#include "Assets\Asset.h"
#include "Systems\Asset_Manager.h"
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

	DT_ENGINE_API ~Asset_Material();
	DT_ENGINE_API Asset_Material();
	DT_ENGINE_API Asset_Material(const std::string &_file, const GLuint & spot);
	DT_ENGINE_API Asset_Material(const std::string(&tx)[MAX_PHYSICAL_IMAGES], const GLuint & spot);
	DT_ENGINE_API static int GetAssetType();
	DT_ENGINE_API void Finalize();

	/****************
	----Variables----
	****************/

	std::string material_filename;
	std::string textures[MAX_PHYSICAL_IMAGES];
	GLuint gl_array_ID;
	vec2 size;
	GLubyte *materialData;
	GLuint mat_spot;


	/*************************
	----Material Functions----
	*************************/

	// Material textures are technically separate files, so we used a custom .mat file to direct which files to use for what texture spot
	// Updates the appropriate supplied @string's with a path to the appropriate file
	DT_ENGINE_API static void getPBRProperties(const string & filename, string & albedo = string(), string & normal = string(), string & metalness = string(), string & roughness = string(), string & height = string(), string & occlusion = string());
};
namespace Asset_Manager {
	DT_ENGINE_API void load_asset(Shared_Asset_Material &user, const std::string(&textures)[6], const bool &threaded = true);
	DT_ENGINE_API void load_asset(Shared_Asset_Material &user, const std::string &material_filename, const bool &threaded = true);
};
#endif // ASSET_MATERIAL