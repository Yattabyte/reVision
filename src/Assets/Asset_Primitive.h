#pragma once
#ifndef	ASSET_PRIMITIVE_H
#define	ASSET_PRIMITIVE_H
#define EXT_PRIMITIVE ".obj"
#define DIRECTORY_PRIMITIVE File_Reader::GetCurrentDir() + "\\Primitives\\"
#define ABS_DIRECTORY_PRIMITIVE(filename) DIRECTORY_PRIMITIVE + filename + EXT_PRIMITIVE

#include "Assets\Asset.h"
#include "Utilities\File_Reader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"
#include <vector>

using namespace glm; 
class Engine;
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


	// Public Methods
	/** Creates a default asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(Engine * engine, Shared_Asset_Primitive & userAsset);
	/** Begins the creation process for this asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(Engine * engine, Shared_Asset_Primitive & userAsset, const string & filename, const bool & threaded = true);
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
	// Private Constructors
	/** Construct the Primitive. */
	Asset_Primitive(const string & filename);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(Engine * engine, Shared_Asset_Primitive & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(Engine * engine, Shared_Asset_Primitive & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_PRIMITIVE_H