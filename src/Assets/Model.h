#pragma once
#ifndef	MODEL_H
#define	MODEL_H

#include "Assets/Mesh.h"
#include "Assets/Material.h"
#include "Managers/ModelManager.h"
#include "assimp/scene.h"
#include "GL/glad/glad.h"
#include "glm/glm.hpp"
#include "glm/geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class ModelManager;
class Model;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Model : public std::shared_ptr<Model> {
public:
	Shared_Model() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Model(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** A 3D geometric mesh meant to be used in 3D rendering. */
class Model : public Asset
{
public:	
	/** Destroy the Model. */
	~Model();
	/** Construct the Model. */
	Model(Engine * engine, const std::string & filename, ModelManager & modelManager);


	// Public Attributes
	Shared_Mesh				m_mesh;
	Shared_Material			m_materialArray;
	GeometryInfo			m_data;
	size_t					m_offset = 0, m_count = 0;
	glm::vec3				m_bboxMin = glm::vec3(0), m_bboxMax = glm::vec3(0), m_bboxCenter = glm::vec3(0);
	float					m_radius = 0.0f;
	ModelManager		*	m_modelManager = nullptr;


private:
	// Private Methods
	/** Calculates a Axis Aligned Bounding Box from a set of vertices.
	Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
	@param	vertices	the vertices of the mesh to derive the AABB from
	@param	minOut	output reference containing the minimum extents of the AABB
	@param	maxOut	output reference containing the maximum extents of the AABB */
	void calculateAABB(const std::vector<SingleVertex> & mesh, glm::vec3 & minOut, glm::vec3 & maxOut, glm::vec3 & centerOut, float & radiusOut);
	/** Create a mesh material, loading the textures as defined by the mesh file itself.
	@note	Used as a failsafe. Mesh importer may not succeed in fetching the directories, and the mesh may not store usable directories.
	@param	engine			the engine being used
	@param	relativePath	the model's filename to use as a guide
	@param	modelMaterial	the material asset to load into
	@param	meshMaterial	the material asset to load into
	@param	sceneMaterial	the scene material to use as a guide */
	void loadMaterial(const std::string & relativePath, Shared_Material & modelMaterial, const std::vector<Material_Strings> & materials);


	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Model;
};

#endif // MODEL_H