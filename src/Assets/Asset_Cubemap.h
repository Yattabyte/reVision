#pragma once
#ifndef	ASSET_CUBEMAP
#define	ASSET_CUBEMAP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_CUBEMAP ".png"
#define DIRECTORY_CUBEMAP File_Reader::GetCurrentDir() + "\\Textures\\Cubemaps\\"
#define ABS_DIRECTORY_CUBEMAP(filename) DIRECTORY_CUBEMAP + filename

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;
class Asset_Cubemap;
typedef shared_ptr<Asset_Cubemap> Shared_Asset_Cubemap;


/**
 * Represents an OpenGL cubemap texture object.
 **/
class DT_ENGINE_API Asset_Cubemap : public Asset
{
public:
	// (de)Constructors
	/** Destroy the Cubemap. */
	~Asset_Cubemap();
	/** Construct the Cubemap. */
	Asset_Cubemap(const std::string & filename);


	// Interface implementations
	/** Returns whether or not this asset has completed finalizing.
	* @return					true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();

	
	// Public Methods
	/** Makes this texture active at a specific texture unit.
	 * @param	texture_unit	the desired texture unit to make this texture active at */
	void bind(const GLuint & texture_unit);

	
	// Public Attributes
	/** @todo make members prefixed with 'm_' */
	GLuint gl_tex_ID;
	vec2 size;
	GLubyte	* pixel_data[6];
	GLsync m_fence;
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists. */
	DT_ENGINE_API void load_asset(Shared_Asset_Cubemap & user, const string & filename, const bool & threaded = true);
};

/**
 * Implements a work order for Cubemap Assets.
 **/
class Cubemap_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Cubemap work order. */
	Cubemap_WorkOrder(Shared_Asset_Cubemap & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Cubemap_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Attributes
	string m_filename;
	Shared_Asset_Cubemap m_asset;
};
#endif // ASSET_CUBEMAP

