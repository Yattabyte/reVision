#pragma once
#ifndef REFLECTOR_TECH_H
#define REFLECTOR_TECH_H

#include "GL\glew.h"
#include "Systems\World\Visibility_Token.h"

/**
 * An interface for specific reflection techniques.
 **/
class Reflector_Tech {
public:
	// (de)Constructors
	/** Virtual Destructor */
	virtual ~Reflector_Tech() {}
	/** Constructor */
	Reflector_Tech() {}


	// Interface Declarations
	/** Perform updates, calculations, and memory writes for indirect reflection lighting
	 * @param	vis_token		the visibility token */
	virtual void updateData(const Visibility_Token & vis_token) = 0;
	/** Apply a pre-lighting pass, calculating things that doesn't depend on the current frame. */
	virtual void applyPrePass() = 0;
	/** Apply the main lighting effect to the given frame. */
	virtual void applyEffect() = 0;
};

#endif //REFLECTOR_TECH_H