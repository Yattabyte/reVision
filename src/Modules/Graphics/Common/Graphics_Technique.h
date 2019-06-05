#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H


/** An interface for core graphics effect techniques. */
class Graphics_Technique {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline virtual ~Graphics_Technique() = default;
	/** Constructor. */
	inline Graphics_Technique() = default;


	// Public Methods
	/** Turn this technique  on or off. 
	@param	state		whether this technique should be on or off. */
	inline void setEnabled(const bool & state) { 
		m_enabled = state; 
	};
	

	// Public Interface
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame. */
	virtual void applyEffect(const float & deltaTime) = 0;


protected:
	// Protected Attributes
	bool m_enabled = true;
};

#endif // GRAPHICS_TECHNIQUE_H