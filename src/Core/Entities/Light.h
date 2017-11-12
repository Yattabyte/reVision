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

class Light : public Entity
{
public:
	/*************
	----Common----
	*************/

	~Light() {};
	Light() {};
	// Tell this entity to register itself into any and all subsystems that it requires
	virtual void registerSelf() {};
	// Tell this entity to un-register itself from any and all subsystems that it required
	virtual void unregisterSelf() {};


	/*************************
	----Light Functions----
	*************************/
	virtual void directPass(const int &vertex_count) {};
	virtual void indirectPass(const int &vertex_count) {};
	
	// Returns a unique ID per class that inherits this
	// Used for categorization in database maps and such
	static int GetLightType() { return -1; };
};

#endif // LIGHT