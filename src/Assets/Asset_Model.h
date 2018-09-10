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


	// Public Attributes
	Shared_Asset_Material				m_materialArray;
	GeometryInfo						m_data;
	std::vector<glm::mat4>				m_boneTransforms;
	std::map<std::string, size_t>		m_boneMap;
	std::vector<Animation>				m_animations;
	Node							*	m_rootNode;
	glm::vec3							m_bboxMin = glm::vec3(0), m_bboxMax = glm::vec3(0), m_bboxCenter = glm::vec3(0);
	float								m_radius = 0.0f;
	size_t								m_offset = 0, m_count = 0;
	ModelManager					*	m_modelManager = nullptr;


protected:
	// Protected Methods
	/** Calculates a Axis Aligned Bounding Box from a set of vertices.
	Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
	@param	vertices	the vertices of the mesh to derive the AABB from
	@param	minOut	output reference containing the minimum extents of the AABB
	@param	maxOut	output reference containing the maximum extents of the AABB */
	void calculateAABB(const std::vector<SingleVertex> & mesh, glm::vec3 & minOut, glm::vec3 & maxOut, glm::vec3 & centerOut, float & radiusOut);
	/** Create a model material, loading the textures as defined by the model file itself.
	@note	Used as a failsafe. Model importer may not succeed in fetching the directories, and the model may not store usable directories.
	@param	engine			the engine being used
	@param	fullDirectory	the model's filename to use as a guide
	@param	modelMaterial	the material asset to load into
	@param	sceneMaterial	the scene material to use as a guide */
	void loadMaterial(Engine * engine, const std::string & fullDirectory, Shared_Asset_Material & modelMaterial, const std::vector<Material> & materials);


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