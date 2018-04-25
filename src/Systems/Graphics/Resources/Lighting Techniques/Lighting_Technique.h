#pragma once
#ifndef LIGHTING_TECHNIQUE
#define LIGHTING_TECHNIQUE

#include "Systems\World\Visibility_Token.h"


/**
 * An interface for solving particular aspects of the lighting equation.
 * @brief In short, these are made to approximate or emulate a particular aspect of how light works.
 **/
class Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Lighting_Technique() {};
	/** Constructor. */
	Lighting_Technique() {};


	// Public Interface Methods
	/** Update underlying data and send it to the GPU. */
	virtual void updateData(const Visibility_Token & vis_token) = 0;
	/** Apply pre-lighting rendering passes. */
	virtual void applyPrePass(const Visibility_Token & vis_token) = 0;
	/** Apply this lighting technique. */
	virtual void applyLighting(const Visibility_Token & vis_token) = 0;
};

#endif // LIGHTING_TECHNIQUE