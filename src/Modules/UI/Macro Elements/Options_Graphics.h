#pragma once
#ifndef OPTIONS_GRAPICS_H
#define OPTIONS_GRAPICS_H

#include "Modules/UI/Macro Elements/Options_Pane.h"


/** A UI element serving as a graphics options menu. */
class Options_Graphics final : public Options_Pane {
public:
	// Public (De)Constructors
	/** Destroy the graphics panel. */
	inline ~Options_Graphics() = default;
	/** Construct a graphics panel.
	@param	engine		reference to the engine to use. */
	explicit Options_Graphics(Engine& engine);


protected:
	// Protected Methods
	/** Set the resolution.
	@param	index	the resolution index to use. */
	void setTextureResolution(const size_t& index);
	/** Set the shadow size.
	@param	index	the shadow size index to use. */
	void setShadowSize(const size_t& index);
	/** Set the reflection size.
	@param	index	the reflection size index to use. */
	void setReflectionSize(const size_t& index);
	/** Set the light bounce quality.
	@param	index	the light bounce quality index to use. */
	void setBounceQuality(const size_t& index);
	/** Turn the bloom on or off.
	@param	b		whether to turn bloom on or off. */
	void setBloom(const bool& b);
	/** Turn the SSAO on or off.
	@param	b		whether to turn SSAO on or off. */
	void setSSAO(const bool& b);
	/** Turn the SSR on or off.
	@param	b		whether to turn SSR on or off. */
	void setSSR(const bool& b);
	/** Turn the FXAA on or off.
	@param	b		whether to turn FXAA on or off. */
	void setFXAA(const bool& b);


	// Protected Attributes
	std::vector<float> m_materialSizes, m_shadowSizes, m_reflectionSizes, m_bounceQuality;
};

#endif // OPTIONS_GRAPICS_H