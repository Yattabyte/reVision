#pragma once
#ifndef GFX_CORE_EFFECT_H
#define GFX_CORE_EFFECT_H


/** An interface for rendering core lighting effects. */
class GFX_Core_Effect {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline virtual ~GFX_Core_Effect() = default;
	/** Constructor. */
	inline GFX_Core_Effect() = default;


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

#endif // GFX_CORE_EFFECT_H