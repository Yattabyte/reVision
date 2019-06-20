#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include <memory>
#include <vector>


class Engine;

/** Represents a series of graphics rendering techniques to apply serially. */
class Graphics_Pipeline {
public:
	// Public (de)Constructors
	/** Destroy this rendering pipeline. */
	inline ~Graphics_Pipeline() = default;
	/** Construct a PBR rendering pipeline. */
	Graphics_Pipeline(Engine * engine);


	// Public Methods
	/***/
	void beginFrame(const float & deltaTime);
	/***/
	void endFrame(const float & deltaTime);
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame. */
	void render(const float & deltaTime, const std::shared_ptr<CameraBuffer> & cameraBuffer, const std::shared_ptr<Graphics_Framebuffers> & gfxFBOS, const std::shared_ptr<RH_Volume> & rhVolume);


protected:
	// Protected Attributes
	Engine * m_engine = nullptr;
	std::vector<Graphics_Technique*> m_techniques;
};

#endif // GRAPHICS_PIPELINE_H