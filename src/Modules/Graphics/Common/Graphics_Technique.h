#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H

#include "Modules/ECS/ecsWorld.h"
#include <memory>
#include <vector>


struct Viewport;
class RH_Volume;

/** An interface for core graphics effect techniques. */
class Graphics_Technique {
public:
	// Public Enumerations
	const enum Technique_Category : unsigned int {
		GEOMETRY = 0b0000'0001,
		PRIMARY_LIGHTING = 0b0000'0010,
		SECONDARY_LIGHTING = 0b0000'0100,
		POST_PROCESSING = 0b0000'1000,
		ALL = 0b1111'1111,
	};


	// Public (de)Constructors
	/** Virtual Destructor. */
	inline virtual ~Graphics_Technique() = default;
	/** Constructor. */
	inline Graphics_Technique(const Technique_Category& category) : m_category(category) {}


	// Public Methods
	/** Retrieve the category this technique belongs to. */
	inline Technique_Category getCategory() const {
		return m_category;
	}
	/** Turn this technique  on or off.
	@param	state			whether this technique should be on or off. */
	inline void setEnabled(const bool& state) {
		m_enabled = state;
	};


	// Public Interface
	/** Prepare this technique for the next frame, swapping any of its buffers.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void prepareForNextFrame(const float& deltaTime) {}
	/** Perform any pre-passes and or update any data needed before rendering this frame.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void updateTechnique(const float& deltaTime, ecsWorld& world) {}
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) {}


protected:
	// Protected Attributes
	bool m_enabled = true;
	Technique_Category m_category;
};

#endif // GRAPHICS_TECHNIQUE_H