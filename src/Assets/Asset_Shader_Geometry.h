#pragma once
#ifndef	ASSET_SHADER_GEOMETRY_H
#define	ASSET_SHADER_GEOMETRY_H

#include "Assets\Asset.h"
#include "Assets\Asset_Shader.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include "GLM\gtc\type_ptr.hpp"
#include <string>


class Engine;
class Asset_Shader_Geometry;
using Shared_Asset_Shader_Geometry = std::shared_ptr<Asset_Shader_Geometry>;

/** An encapsulation of a vertex/geometry/fragment OpenGL shader program, extending Asset_Shader. */
class Asset_Shader_Geometry : public Asset_Shader
{
public:	
	/** Destroy the Shader. */
	~Asset_Shader_Geometry();
	/** Construct the Shader. */
	Asset_Shader_Geometry(const std::string & filename);


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Shader_Geometry Create(Engine * engine, const std::string & filename, const bool & threaded = true);

	
	// Public Attributes
	ShaderObj m_geometryShader = ShaderObj(GL_GEOMETRY_SHADER);


protected:
	// Protected Methods
	// Interface Implementation
	virtual const bool initShaders(Engine * engine, const std::string & relativePath) override;


private:
	// Private Methods
	// Interface Implementation
	void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_GEOMETRY_H
