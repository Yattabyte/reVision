#pragma once
#ifndef GEOMETRY_COMPONENT
#define GEOMETRY_COMPONENT
#ifdef	ENGINE_EXPORT
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

	/** Tests if this object is within the viewing frustum of the camera.
	 * @brief				a test of general visibility (excluding obstruction of other objects). 
	 * @param	PMatrix		the projection matrix of the camera
	 * @param	VMatrix		the viewing matrix of the camera
	 * @return				true if this object is within the viewing frustum of the camera, false otherwise */
	virtual bool isVisible(const mat4 & PMatrix, const mat4 & VMatrix) = 0;


protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Geometry_Component() {};
	/** Constructor. Takes in component ID and parent ID. */
	Geometry_Component(const ECShandle & id, const ECShandle & pid) : Component(id, pid) {};
};

#endif // GEOMETRY_COMPONENT