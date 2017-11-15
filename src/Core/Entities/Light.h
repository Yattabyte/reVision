/*
	Light

	- An abstract class to be expanded on by all lighting entities
	- To be used in the Lighting Manager
*/

#pragma once
#ifndef LIGHT
#define LIGHT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include "glm\glm.hpp"

using namespace glm;

class Visibility_Token;
class Light : public Entity
{
public:
	/*************
	----Common----
	*************/
	// Tell this entity to register itself into any and all subsystems that it requires
	virtual void registerSelf() {};
	// Tell this entity to un-register itself from any and all subsystems that it required
	virtual void unregisterSelf() {};


	/*************************
	----Light Functions----
	*************************/
	// Direct lighting pass
	virtual void directPass(const int &vertex_count) {};
	// Indirect lighting pass
	virtual void indirectPass(const int &vertex_count) {};
	// Shadow lighting pass
	virtual void shadowPass(const Visibility_Token &vis_token) const {};	
	// Returns a unique ID per class that inherits this
	// Used for categorization in database maps and such
	static int GetLightType() { return -1; };
	// Returns whther or not this object should render
	virtual bool shouldRender(const mat4 &PVMatrix) { return false; };
};

#endif // LIGHT