#pragma once
#ifndef EFFECT_BASE_H
#define EFFECT_BASE_H


/** An interface for applying post processing effects in the rendering pipeline. */
class Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Effect_Base() {};
	/** Constructor. */
	Effect_Base() {}


	// Public Methods
	/** Apply this lighting technique. 
	@param	deltaTime	the amount of time passed since last frame. */
	virtual void applyEffect(const float & deltaTime) = 0;
};

#endif // EFFECT_BASE_H