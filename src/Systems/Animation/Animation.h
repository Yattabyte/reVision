#pragma once
#ifndef SYSTEM_ANIMATION
#define SYSTEM_ANIMATION
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"

class EnginePackage;


/**
 * An engine system that controls animation-supporting objects.
 **/
class DT_ENGINE_API System_Animation : public System
{
public:
	// (de)Constructors
	/** Destroy the animation system. */
	~System_Animation() {}
	/** Construct the animation system. */
	System_Animation() {}


	// Interface Implementations
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};
};

#endif // SYSTEM_ANIMATION