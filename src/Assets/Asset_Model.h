#pragma once
#ifndef	ASSET_MODEL_H
#define	ASSET_MODEL_H

#include "Assets\Asset.h"
#include "Assets\Asset_Material.h"
#include "Managers\ModelManager.h"
#include "assimp\scene.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class ModelManager;
class Asset_Model;
using Shared_Asset_Model = std::shared_ptr<Asset_Model>;

/** A 3D geometric mesh meant to be used in 3D rendering. */
class Asset_Model : public Asset
{
public:	
	/** Destroy the Model. */
	~Asset_Model();


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */	
	static Shared_Asset_Model Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	/** Returns the material ID for a skin given an index into this list
	@note					Clamps to the skin list size, so it won't go out of bounds
	@param	index			into this model's skin list. 
	@return					index into the master material list in which this skin can be found at */	
	GLuint getSkinID(const unsigned int & desired);


	// Public Attributes
	std::vector<Shared_Asset_Material>		m_skins;
	GeometryInfo							m_data;
	std::vector<BoneTransform>				m_boneTransforms;
	std::map<std::string, size_t>			m_boneMap;
	std::vector<Animation>					m_animations;
	Node								*	m_rootNode;
	glm::vec3								m_bboxMin = glm::vec3(0), m_bboxMax = glm::vec3(0), m_bboxCenter = glm::vec3(0);
	float									m_radius = 0.0f;
	size_t									m_offset = 0, m_count = 0;
	ModelManager						*	m_modelManager = nullptr;


private:
	// Private Constructors
	/** Construct the Model. */
	Asset_Model(const std::string & filename, ModelManager & modelManager);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine) override;
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MODEL_H