#pragma once
#ifndef	ASSET_PRIMITIVE_H
#define	ASSET_PRIMITIVE_H

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\common.hpp"
#include <vector>


class Engine;
class Asset_Primitive;
typedef std::shared_ptr<Asset_Primitive> Shared_Asset_Primitive;

/** A basic geometric shape to be used in basic visual processing, such as a quad or a sphere. */
class Asset_Primitive : public Asset
{
public:
	/** Destroy the Primitive. */
	~Asset_Primitive();


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread 
	@return					the desired asset */
	static Shared_Asset_Primitive Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	/** Generates a vertex array object, formed to match primitives' object data.
	@return					a vertex array object resident on the GPU */
	static const GLuint Generate_VAO();
	/** Updates a vertex array object's state with this primitives' data. 
	@brief					using the supplied vertex array object, updates its internal data on the GPU with this primitives underlying data.
	@param	vaoID			the vertex array object's ID on the GPU */	
	void updateVAO(const GLuint & vaoID);
	/** Returns the vertex-count of this object. 
	@return					vertex-count of this object */
	size_t getSize();
	
	
	// Public Attributes
	GLuint m_buffers[2];
	std::vector<glm::vec3> m_dataVertex;
	std::vector<glm::vec2> m_dataUV;


private:
	// Private Constructors
	/** Construct the Primitive. */
	Asset_Primitive(const std::string & filename);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & fullDirectory);
	virtual void finalize(Engine * engine);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_PRIMITIVE_H