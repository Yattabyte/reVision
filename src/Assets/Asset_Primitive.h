#pragma once
#ifndef	ASSET_PRIMITIVE
#define	ASSET_PRIMITIVE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_PRIMITIVE ".obj"
#define DIRECTORY_PRIMITIVE File_Reader::GetCurrentDir() + "\\Primitives\\"
#define ABS_DIRECTORY_PRIMITIVE(filename) DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"
#include <vector>

using namespace glm;
class Asset_Primitive;
typedef shared_ptr<Asset_Primitive> Shared_Asset_Primitive;


/**
 * A basic geometric shape to be used in basic visual processing, such as a quad or a sphere.
 **/
class DT_ENGINE_API Asset_Primitive : public Asset
{
public:
	// (de)Constructors
	/** Destroy the Primitive. */
	~Asset_Primitive();
	/** Construct the Primitive. */
	Asset_Primitive(const string & filename);


	// Methods
	/** @todo delete */
	static int Get_Asset_Type();
	/** Generates a vertex array object, formed to match primitives' object data.
	 * @return	a vertex array object resident on the GPU */
	static GLuint Generate_VAO();
	/** Updates a vertex array object's state with this primitives' data. 
	 * @brief	using the supplied vertex array object, updates its internal data on the GPU with this primitives underlying data.
	 * @param	vaoID	the vertex array object's ID on the GPU */	
	void updateVAO(const GLuint & vaoID);
	/** Returns the vertex-count of this object. 
	 * @return	vertex-count of this object */
	size_t getSize();
	

	// Interface Implementation
	/** Returns whether or not this asset has completed finalizing.
	 * @return	true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();


	// Attributes
	GLuint buffers[2];
	vector<vec3> data;
	vector<vec2> uv_data;
	GLsync m_fence;
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists */
	DT_ENGINE_API void load_asset(Shared_Asset_Primitive & user, const string & filename, const bool & threaded = true);
};

/**
 * Implements a work order for Primitive Assets.
 **/
class Primitive_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Primitive work order */
	Primitive_WorkOrder(Shared_Asset_Primitive & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Primitive_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Attributes
	string m_filename;
	Shared_Asset_Primitive m_asset;
};
#endif // ASSET_PRIMITIVE