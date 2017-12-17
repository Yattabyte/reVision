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
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_PRIMITIVE ".obj"
#define DIRECTORY_PRIMITIVE FileReader::GetCurrentDir() + "\\Primitives\\"
#define ABS_DIRECTORY_PRIMITIVE(filename) DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\FileReader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"
#include <vector>

using namespace glm;

class Asset_Primitive;
typedef shared_ptr<Asset_Primitive> Shared_Asset_Primitive;
class DT_ENGINE_API Asset_Primitive : public Asset
{
public:
	/*************
	----Common----
	*************/

	~Asset_Primitive();
	Asset_Primitive();
	Asset_Primitive(const string &_filename);
	static int GetAssetType();

	
	/****************
	----Variables----
	****************/

	GLuint buffers[2];
	string filename;
	vector<vec3> data;
	vector<vec2> uv_data;


	/**************************
	----Primitive Functions----
	**************************/
	
	// Generates a vertex array object, formed to match primitives' object data
	static GLuint GenerateVAO();
	// Updates a vertex array object's state with this objects' data
	void UpdateVAO(const GLuint &vaoID);
	// Returns the vertex-count of this object
	size_t GetSize();
};

namespace Asset_Loader {
	DT_ENGINE_API void load_asset(Shared_Asset_Primitive &user, const string & filename, const bool &threaded = true);
};

class Primitive_WorkOrder : public Work_Order {
public:
	Primitive_WorkOrder(Shared_Asset_Primitive &asset, const std::string &filename) : m_asset(asset), m_filename(filename) {};
	~Primitive_WorkOrder() {};
	virtual void Initialize_Order();
	virtual void Finalize_Order();

private:
	std::string m_filename;
	Shared_Asset_Primitive m_asset;
};
#endif // ASSET_PRIMITIVE