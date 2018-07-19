#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "Systems\System_Interface.h"


class System_World;

/**
 * An engine system that controls animation-supporting components.
 **/
class System_Animation : public System
{
public:
	// (de)Constructors
	/** Destroy the animation system. */
	~System_Animation() {}
	/** Construct the animation system. */
	System_Animation() {}


	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {}
};

#endif // ANIMATOR_H