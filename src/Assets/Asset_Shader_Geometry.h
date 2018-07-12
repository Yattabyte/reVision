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
typedef std::shared_ptr<Asset_Shader_Geometry> Shared_Asset_Shader_Geometry;

/**
 * An encapsulation of an OpenGL shader program.\n
 * Supports vertex, fragment, and geometry shaders.\n
 * Also provides support for explicitly setting uniform values for a given attribute location.
 **/
class Asset_Shader_Geometry : public Asset_Shader
{
public:	
	/** Destroy the Shader. */
	~Asset_Shader_Geometry();


	// Public Methods
	/** Creates a default asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(Engine * engine, Shared_Asset_Shader_Geometry & userAsset);
	/** Begins the creation process for this asset.
	 * @param	engine			the engine being used
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(Engine * engine, Shared_Asset_Shader_Geometry & userAsset, const std::string & filename, const bool & threaded = true);

	
	// Public Attributes
	GLuint m_glGeometryID; // OpenGL ID's
	std::string m_geometryText; // Text Data


private:
	// Private Constructors
	/** Construct the Shader. */
	Asset_Shader_Geometry(const std::string & filename);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(Engine * engine, Shared_Asset_Shader_Geometry & userAsset, const std::string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(Engine * engine, Shared_Asset_Shader_Geometry & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SHADER_GEOMETRY_H
