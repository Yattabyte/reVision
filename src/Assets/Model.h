#pragma once
#ifndef	MODEL_H
#define	MODEL_H

#include "Assets/Mesh.h"
#include "Assets/Material.h"
#include "glm/glm.hpp"


// Forward Declarations
class Model;

/** Shared version of a Model asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Model final : public std::shared_ptr<Model> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Model() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Model(Engine& engine, const std::string& filename, const bool& threaded = true);
};

/** A 3D mesh formated for model rendering.
Contains a formatted 3D mesh to be used for 3D rendering, including information about its materials and bounding box.
@note	owns 1 Shared_Mesh object.
@note	owns 1 Shared_Material object. */
class Model final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Model. */
	inline ~Model() noexcept = default;
	/** Construct the Model.
	@param	engine		reference to the engine to use. 
	@param	filename	the asset file name (relative to engine directory). */
	Model(Engine& engine, const std::string& filename);


	// Public Attributes
	Shared_Mesh				m_mesh;
	Shared_Material			m_materialArray;
	GeometryInfo			m_data;
	glm::vec3				m_bboxMin = glm::vec3(0), m_bboxMax = glm::vec3(0), m_bboxCenter = glm::vec3(0), m_bboxScale = glm::vec3(1);
	float					m_radius = 0.0f;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Model(Model&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Model(const Model&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Model& operator =(Model&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Model& operator =(const Model&) noexcept = delete;


	// Private Methods
	/** Calculates a Axis Aligned Bounding Box from a set of vertices.
	Returns it as updated minimum and maximum values &minOut and &maxOut respectively.
	@param	vertices	the vertices of the mesh to derive the AABB from.
	@param	minOut		output reference containing the minimum extents of the AABB.
	@param	maxOut		output reference containing the maximum extents of the AABB.
	@param	scaleOut	output reference containing the scale of the AABB.
	@param	centerOut	output reference containing the center of the AABB.
	@param	radiusOut	output reference containing the radius of the AABB converted to a sphere. */
	void static calculateAABB(const std::vector<SingleVertex>& mesh, glm::vec3& minOut, glm::vec3& maxOut, glm::vec3& scaleOut, glm::vec3& centerOut, float& radiusOut);
	/** Create a mesh material, loading the textures as defined by the mesh file itself.
	@note	Used as a failsafe. Mesh importer may not succeed in fetching the directories, and the mesh may not store usable directories.
	@param	relativePath	the model's filename to use as a guide.
	@param	modelMaterial	the material asset to load into.
	@param	materials		the scene material to use as a guide. */
	void loadMaterial(const std::string& relativePath, Shared_Material& modelMaterial, const std::vector<Material_Strings>& materials);


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Model;
};

#endif // MODEL_H