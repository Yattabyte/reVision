#pragma once
#ifndef	ASSET_PRIMITIVE_H
#define	ASSET_PRIMITIVE_H

#include "Assets\Asset.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"
#include <vector>


class Engine;
class Asset_Primitive;
struct Single_Primitive_Vertex;
using Shared_Asset_Primitive = std::shared_ptr<Asset_Primitive>;

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
	/** Returns the vertex-count of this object. 
	@return					vertex-count of this object */
	size_t getSize();
	
	
	// Public Attributes
	GLuint m_uboID = 0, m_vaoID = 0;
	std::vector<Single_Primitive_Vertex> m_data;


private:
	// Private Constructors
	/** Construct the Primitive. */
	Asset_Primitive(const std::string & filename);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine) override;
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};
struct Single_Primitive_Vertex {
	glm::vec3 vertex;
	glm::vec2 uv;
};

#endif // ASSET_PRIMITIVE_H