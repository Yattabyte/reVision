#pragma once
#ifndef SYSTEM_LOGIC_H
#define SYSTEM_LOGIC_H

#include "Systems\System_Interface.h"
#include "Utilities\Transform.h"

class Engine;


/**
 * An engine system that controls updating game-state every tick
 * @todo	add physics and stuff
 **/
class System_Logic : public System
{
public:
	// (de)Constructors
	/** Destroy the logic system. */
	~System_Logic();
	/** Construct the logic system. */
	System_Logic();


	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


private:
	// Private Attributes
	Transform m_transform;
	vec3 m_rotation;
};

#endif // SYSTEM_LOGIC_H