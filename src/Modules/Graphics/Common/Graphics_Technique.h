#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H

#include "Modules/World/ECS/ecsSystem.h"


/** An interface for core graphics effect techniques. */
class Graphics_Technique : public BaseECSSystem {
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
	virtual void applyEffect(const float & deltaTime) {}
	/** Tick this system by deltaTime, passing in all the components matching this system's requirements.
	@param	deltaTime		the amount of time which passed since last update
	@param	components		the components to update. */
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {};


protected:
	// Protected Attributes
	bool m_enabled = true;
};

struct Null_Component : public ECSComponent<Null_Component> {
};

#endif // GRAPHICS_TECHNIQUE_H