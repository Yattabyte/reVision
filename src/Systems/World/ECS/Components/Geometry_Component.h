#pragma once
#ifndef GEOMETRY_COMPONENT
#define GEOMETRY_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Components\Component.h"
#include "glm\glm.hpp"

using namespace glm;


/**
 * An interface for renderable components with a 3D mesh to implement.
 **/
class DT_ENGINE_API Geometry_Component : protected Component
{
public:
	// Public Methods
	/** Renders the model to the current framebuffer. */
	virtual void draw() = 0;
	/** Retrieves the offset and vertex count of this geometry.
	 * @return offset and vertex count as a vector */
	virtual const ivec2 getDrawInfo() const = 0;


protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Geometry_Component() {};
	/** Constructor. Takes in component ID and parent ID. */
	Geometry_Component(const ECShandle & id, const ECShandle & pid) : Component(id, pid) {};
};

#endif // GEOMETRY_COMPONENT