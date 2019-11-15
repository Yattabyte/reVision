#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H

#include "Modules/ECS/ecsWorld.h"
#include <memory>
#include <vector>


struct Viewport;

/** An interface for core graphics effect techniques. */
class Graphics_Technique {
public:
	// Public Enumerations
	enum class Technique_Category : unsigned int {
		GEOMETRY = 0b0000'0001,
		PRIMARY_LIGHTING = 0b0000'0010,
		SECONDARY_LIGHTING = 0b0000'0100,
		POST_PROCESSING = 0b0000'1000,
		ALL = 0b1111'1111,
	};


	// Public (De)Constructors
	/** Virtual Destructor. */
	inline virtual ~Graphics_Technique() = default;
	/** Constructor. */
	inline explicit Graphics_Technique(const Technique_Category& category) noexcept : m_category(category) {}


	// Public Methods
	/** Retrieve the category this technique belongs to. */
	inline Technique_Category getCategory() const noexcept {
		return m_category;
	}
	/** Turn this technique  on or off.
	@param	state			whether this technique should be on or off. */
	inline void setEnabled(const bool& state) noexcept {
		m_enabled = state;
	};


	// Public Interface
	/** Prepare this technique for the next frame, swapping any of its buffers.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void clearCache(const float& deltaTime) noexcept {}
	/** Update any data needed before rendering this frame.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void updateCache(const float& deltaTime, ecsWorld& world) noexcept {}
	/** Perform any pre-requisite rendering passes.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void updatePass(const float& deltaTime) noexcept {}
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept {}


protected:
	// Protected Attributes
	bool m_enabled = true;
	Technique_Category m_category;
};

#endif // GRAPHICS_TECHNIQUE_H