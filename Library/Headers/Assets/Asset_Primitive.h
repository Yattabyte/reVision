/*
	Asset_Primitive
	
	- A basic geometry shape with no complex attributes assigned to it.
	- Intended to be used in rendering scenarios where basic shapes are needed, such as:
		- a full screen quad
		- a point-light bounding sphere
		- a spot-light bounding cone
	- No coresponding texture, just raw geometric data is sought after
	- Shapes saved on disk
*/

#pragma once
#ifndef	ASSET_PRIMITIVE
#define	ASSET_PRIMITIVE
#ifdef	ASSET_PRIMITIVE_EXPORT
#define	ASSET_PRIMITIVE_API __declspec(dllexport)
#else
#define	ASSET_PRIMITIVE_API __declspec(dllimport)
#endif
#define EXT_PRIMITIVE ".obj"
#define DIRECTORY_PRIMITIVE getCurrentDir() + "\\Primitives\\"
#define ABS_DIRECTORY_PRIMITIVE(filename) DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE

#include "Managers\Asset_Manager.h"
#include "GL\glew.h"
#include "GLM\common.hpp"
#include <vector>

using namespace glm;

class Asset_Primitive;
typedef shared_ptr<Asset_Primitive> Shared_Asset_Primitive;
class Asset_Primitive : public Asset
{
public:
	/*************
	----Common----
	*************/

	ASSET_PRIMITIVE_API ~Asset_Primitive();
	ASSET_PRIMITIVE_API Asset_Primitive();
	ASSET_PRIMITIVE_API Asset_Primitive(const string &_filename);
	ASSET_PRIMITIVE_API static int GetAssetType();
	ASSET_PRIMITIVE_API virtual void Finalize();
	
	/****************
	----Variables----
	****************/

	GLuint ID;
	string filename;
	vector<vec3> data;
	vector<vec2> uv_data;

	/**************************
	----Primitive Functions----
	**************************/

	// Binds this object's VAO to the current rendering context
	ASSET_PRIMITIVE_API void Bind();
	// Unbinds any previously bound VAO from the current rendering context (binds null)
	ASSET_PRIMITIVE_API static void Unbind();
	// Renders this object into the currently bound framebuffer.
	ASSET_PRIMITIVE_API void Draw();
	// Returns the vertex-count of this object
	ASSET_PRIMITIVE_API size_t GetSize() const;
};
#endif // ASSET_PRIMITIVE