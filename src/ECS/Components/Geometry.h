#pragma once
#ifndef GEOMETRY_COMPONENT_H
#define GEOMETRY_COMPONENT_H

#include "ECS\Components\Component.h"
#include "glm\glm.hpp"


/**
 * An interface for renderable components with a 3D mesh to implement.
 **/
class Geometry_C : public Component
{
public:
	// Public Methods
	/** Retrieves the offset and vertex count of this geometry.
	 * @return		offset and vertex count as a std::vector */
	virtual const glm::ivec2 getDrawInfo() const = 0;
	/** Retrieves the mesh size of this piece of geometry. 
	 * @return		the vertex count of this mesh */
	virtual const unsigned int getMeshSize() const = 0;


protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Geometry_C() {};
	/** Constructor.
	 * @param	engine	the engine */
	Geometry_C(Engine * engine) : Component(engine) {};
};

#endif // GEOMETRY_COMPONENT_H