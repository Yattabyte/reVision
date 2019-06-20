#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/RH_Volume.h"


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
	@param	state			whether this technique should be on or off. */
	inline void setEnabled(const bool & state) { 
		m_enabled = state; 
	};
	/** Set the common underlying data that all graphics techniques share.
	@param	cameraBuffer	the buffer holding camera data.
	@param	gfxFBOS			the core framebuffers to render into. 
	@param	rhVolume		the radiance-hints buffer to use for indirect lighting. */
	inline void setViewingParameters(const std::shared_ptr<CameraBuffer> & cameraBuffer, const std::shared_ptr<Graphics_Framebuffers> & gfxFBOS, const std::shared_ptr<RH_Volume> & rhVolume) {
		m_cameraBuffer = cameraBuffer;
		m_gfxFBOS = gfxFBOS;
		m_volumeRH = rhVolume;
	}
	

	// Public Interface
	/***/
	inline virtual void beginFrame(const float & deltaTime) {}
	/***/
	inline virtual void endFrame(const float & deltaTime) {}
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void renderTechnique(const float & deltaTime) {}


protected:
	// Protected Attributes
	bool m_enabled = true;
	std::shared_ptr<CameraBuffer> m_cameraBuffer;
	std::shared_ptr<Graphics_Framebuffers> m_gfxFBOS;
	std::shared_ptr<RH_Volume> m_volumeRH;
};

#endif // GRAPHICS_TECHNIQUE_H