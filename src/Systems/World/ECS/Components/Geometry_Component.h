#pragma once
#ifndef GEOMETRY_COMPONENT_H
#define GEOMETRY_COMPONENT_H

#include "Systems\World\ECS\Components\Component.h"
#include "glm\glm.hpp"

using namespace glm;


/**
 * An interface for renderable components with a 3D mesh to implement.
 **/
class Geometry_Component : protected Component
{
public:
	// Public Methods
	/** Retrieves the offset and vertex count of this geometry.
	 * @return		offset and vertex count as a vector */
	virtual const ivec2 getDrawInfo() const = 0;
	/** Retrieves the mesh size of this piece of geometry. 
	 * @return		the vertex count of this mesh */
	virtual const unsigned int getMeshSize() const = 0;


protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Geometry_Component() {};
	/** Constructor. */
	Geometry_Component() {};
};

#endif // GEOMETRY_COMPONENT_H