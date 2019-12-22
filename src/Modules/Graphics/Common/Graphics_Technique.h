#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H

#include "Modules/ECS/ecsWorld.h"
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
	explicit Graphics_Technique(const Technique_Category& category) noexcept;


	// Public Methods
	/** Retrieve the category this technique belongs to. */
	Technique_Category getCategory() const noexcept;
	/** Turn this technique  on or off.
	@param	state			whether this technique should be on or off. */
	void setEnabled(const bool& state) noexcept;


	// Public Interface
	/** Prepare this technique for the next frame, swapping any of its buffers.
	@param	deltaTime		the amount of time passed since last frame. */
	virtual void clearCache(const float& deltaTime);
	/** Update any data needed before rendering this frame.
	@param	deltaTime		the amount of time passed since last frame. 
	@param	world			the ecsWorld to source data from. */
	virtual void updateCache(const float& deltaTime, ecsWorld& world);
	/** Perform any pre-requisite rendering passes.
	@param	deltaTime		the amount of time passed since last frame. */
	virtual void updatePass(const float& deltaTime);
	/** Apply this lighting technique.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render from.
	@param	perspectives	the viewing perspectives to render from. */
	virtual void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives);


protected:
	// Protected Attributes
	bool m_enabled = true;
	Technique_Category m_category;
};

#endif // GRAPHICS_TECHNIQUE_H