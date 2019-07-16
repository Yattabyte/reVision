#pragma once
#ifndef	PRIMITIVE_H
#define	PRIMITIVE_H

#include "Assets/Mesh.h"
#include "Utilities/GL/glad/glad.h"
#include "glm/glm.hpp"
#include <vector>


class Engine;
class Primitive;
struct Single_Primitive_Vertex;

/** Shared version of a Primitive asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Primitive : public std::shared_ptr<Primitive> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Primitive() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Primitive(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** A basic untextued 3D model used in visual processing.
Represents a more basic 3D model with a material, and wraps an OpenGL vertex array & buffer object.
Typically used to load quads/spheres for fire & forget events.
@note	owns 1 Shared_Mesh object. */
class Primitive : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Primitive. */
	~Primitive();
	/** Construct the Primitive.
	@param	engine		the engine to use.
	@param	filename	the asset file name (relative to engine directory). */
	Primitive(Engine * engine, const std::string & filename);


	// Public Methods
	/** Returns the vertex-count of this object. 
	@return					vertex-count of this object. */
	size_t getSize() const;
	
	
	// Public Attributes
	Shared_Mesh m_mesh;
	std::vector<Single_Primitive_Vertex> m_data;
	GLuint m_uboID = 0, m_vaoID = 0;


private:
	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Primitive;
};

#endif // PRIMITIVE_H