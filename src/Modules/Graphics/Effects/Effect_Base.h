#pragma once
#ifndef EFFECT_BASE_H
#define EFFECT_BASE_H


/** An interface for applying post processing effects in the rendering pipeline. */
class Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Effect_Base() = default;
	/** Constructor. */
	Effect_Base() = default;


	// Public Methods
	/** Return whether or not this effect is enabled*/
	inline const bool isEnabled() const { return m_enabled; };
	// Public Interface
	/** Apply this lighting technique. 
	@param	deltaTime	the amount of time passed since last frame. */
	virtual void applyEffect(const float & deltaTime) = 0;


protected:
	// Protected Attributes
	bool m_enabled = true;
};

#endif // EFFECT_BASE_H