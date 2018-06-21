#pragma once
#ifndef	ASSET_PRIMITIVE_H
#define	ASSET_PRIMITIVE_H
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
class Asset_Primitive : public Asset
{
public:
	/** Destroy the Primitive. */
	~Asset_Primitive();


	/** Attempts to create an asset from disk or share one if it already exists */
	static void Create(Shared_Asset_Primitive & userAsset, const string & filename, const bool & threaded = true);


	// Public Methods
	/** Generates a vertex array object, formed to match primitives' object data.
	 * @return			a vertex array object resident on the GPU */
	static GLuint Generate_VAO();
	/** Updates a vertex array object's state with this primitives' data. 
	 * @brief			using the supplied vertex array object, updates its internal data on the GPU with this primitives underlying data.
	 * @param	vaoID	the vertex array object's ID on the GPU */	
	void updateVAO(const GLuint & vaoID);
	/** Returns the vertex-count of this object. 
	 * @return			vertex-count of this object */
	size_t getSize();
	
	
	// Public Attributes
	GLuint m_buffers[2];
	vector<vec3> m_dataVertex;
	vector<vec2> m_dataUV;


private:
	/** Construct the Primitive. */
	Asset_Primitive(const string & filename);
	friend class Asset_Manager;
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
	// Private Attributes
	string m_filename;
	Shared_Asset_Primitive m_asset;
};
#endif // ASSET_PRIMITIVE_H