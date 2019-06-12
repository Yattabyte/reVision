#pragma once
#ifndef GFX_PP_EFFECT_H
#define GFX_PP_EFFECT_H


/** An interface for rendering post-processing effects. */
class GFX_PP_Effect {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline virtual ~GFX_PP_Effect() = default;
	/** Constructor. */
	inline GFX_PP_Effect() = default;


	// Public Methods
	/** Return whether or not this effect is enabled*/
	inline const bool isEnabled() const { return m_enabled; };
	// Public Interface
	/** Apply this lighting technique. 
	@param	deltaTime	the amount of time passed since last frame. */
	virtual void applyTechnique(const float & deltaTime) = 0;


protected:
	// Protected Attributes
	bool m_enabled = true;
};

#endif // GFX_PP_EFFECT_H