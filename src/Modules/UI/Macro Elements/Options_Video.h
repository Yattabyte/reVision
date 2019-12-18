#pragma once
#ifndef OPTIONS_VIDEO_H
#define OPTIONS_VIDEO_H

#include "Modules/UI/Macro Elements/Options_Pane.h"
#include "glm/glm.hpp"


/** A UI element serving as a video options menu. */
class Options_Video final : public Options_Pane {
public:
	// Public (De)Constructors
	/** Destroy the video pane. */
	inline ~Options_Video() noexcept = default;
	/** Construct a video pane.
	@param	engine		reference to the engine to use. */
	explicit Options_Video(Engine& engine);


protected:
	// Protected Attributes
	std::vector<glm::ivec3> m_resolutions;
};

#endif // OPTIONS_VIDEO_H