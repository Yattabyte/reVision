#pragma once
#ifndef	ASSET_CUBEMAP_H
#define	ASSET_CUBEMAP_H
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
class Asset_Cubemap : public Asset
{
public:
	/** Destroy the Cubemap. */
	~Asset_Cubemap();
	/** Attempts to create an asset from disk or share one if it already exists. */
	static void Create(Shared_Asset_Cubemap & userAsset, const string & filename, const bool & threaded = true);

	
	// Public Methods
	/** Makes this texture active at a specific texture unit.
	 * @param	texture_unit	the desired texture unit to make this texture active at */
	void bind(const unsigned int & texture_unit);

	
	// Public Attributes
	GLuint m_glTexID;
	vec2 m_size;
	GLubyte	* m_pixelData[6];


private:
	/** Construct the Cubemap. */
	Asset_Cubemap(const std::string & filename);
	friend class Asset_Manager;
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
#endif // ASSET_CUBEMAP_H

