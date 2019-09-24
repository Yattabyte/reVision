#pragma once
#ifndef	PRIMITIVE_H
#define	PRIMITIVE_H

#include "Assets/Mesh.h"
#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"
#include <vector>


class Engine;
class Auto_Model;
struct Single_Primitive_Vertex;

/** Shared version of an Auto_Model asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Auto_Model final : public std::shared_ptr<Auto_Model> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Auto_Model() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Auto_Model(Engine* engine, const std::string& filename, const bool& threaded = true);
};

/** A basic 3D model used in visual processing.
Represents a more basic 3D model, such as a sphere or a quad, and wraps an OpenGL vertex array & buffer object.
Typically used to load quads/spheres for fire & forget events.
@note	owns 1 Shared_Mesh object. */
class Auto_Model final : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Auto_Model. */
	~Auto_Model();
	/** Construct the Auto_Model.
	@param	engine		the engine to use.
	@param	filename	the asset file name (relative to engine directory). */
	Auto_Model(Engine* engine, const std::string& filename);


	// Public Methods
	/** Returns the vertex-count of this object.
	@return					vertex-count of this object. */
	size_t getSize() const;
	/** Bind this model's VAO to the currently active GL context. */
	void bind();


	// Public Attributes
	Shared_Mesh m_mesh;
	std::vector<Single_Primitive_Vertex> m_data;
	GLuint m_vboID = 0, m_vaoID = 0;


private:
	// Private Interface Implementation
	virtual void initialize() override final;


	// Private Attributes
	friend class Shared_Auto_Model;
};

#endif // PRIMITIVE_H