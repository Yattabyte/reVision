/*
	Geometry

	- An abstract class to be expanded on by all renderable entities
	- To be used in the Geometry Manager
*/

#pragma once
#ifndef GEOMETRY
#define GEOMETRY
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include "Utilities\Frustum.h"

class Geometry /*: public Entity*/
{
public:
	/*************
	----Common----
	*************/

	~Geometry() {};
	Geometry() {};
	// Tell this entity to register itself into any and all subsystems that it requires
	virtual void registerSelf() {};
	// Tell this entity to un-register itself from any and all subsystems that it required
	virtual void unregisterSelf() {};


	/*************************
	----Geometry Functions----
	*************************/
	
	// Returns a unique ID per class that inherits this
	// Used for categorization in database maps and such
	static int GetGeometryType() { return -1; };
	// Returns whther or not this object should render
	virtual bool shouldRender(const mat4 &PVMatrix) { return true; };
	// Render this geometry
	virtual void geometryPass() const {};
};

#endif // GEOMETRY