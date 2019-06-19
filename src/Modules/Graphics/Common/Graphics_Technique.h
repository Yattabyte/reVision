#pragma once
#ifndef GRAPHICS_TECHNIQUE_H
#define GRAPHICS_TECHNIQUE_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/RH_Volume.h"


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
	inline virtual void beginWriting() {}
	/***/
	inline virtual void endWriting() {}
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void applyTechnique(const float & deltaTime) {}
	/** Tick this system by deltaTime, passing in all the components matching this system's requirements.
	@param	deltaTime		the amount of time which passed since last update
	@param	components		the components to update. */
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override{};


protected:
	// Protected Attributes
	bool m_enabled = true;
	std::shared_ptr<CameraBuffer> m_cameraBuffer;
	std::shared_ptr<Graphics_Framebuffers> m_gfxFBOS;
	std::shared_ptr<RH_Volume> m_volumeRH;
};

struct Null_Component : public ECSComponent<Null_Component> {
};

#endif // GRAPHICS_TECHNIQUE_H