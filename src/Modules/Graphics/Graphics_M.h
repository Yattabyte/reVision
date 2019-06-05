#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/Graphics_Framebuffers.h"
#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/VisualFX.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/MappedChar.h"


/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this graphics rendering module. */
	~Graphics_Module();
	/** Construct a graphics rendering module. */
	inline Graphics_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Render a single frame.
	@param	deltaTime	the amount of time passed since last frame */
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods	
	/** Update the data for the specified camera. */
	void updateCamera();
	/** Returns the active camera's data buffer.
	@return					the active camera's data buffer. */
	std::shared_ptr<CameraBuffer> getCameraBuffer() const;
	/** Returns the lighting buffer's FBO ID. 
	@return					the lighting buffer FBO ID. */
	inline GLuint getLightingFBOID() const {
		return m_graphicsFBOS->m_fbos["LIGHTING"].first;
	};
	/** Returns the lighting buffer's texture ID.
	@return					the lighting buffer texture ID. */
	inline GLuint getLightingTexID() const {
		return std::get<0>(m_graphicsFBOS->m_fbos["LIGHTING"].second.front());
	};

	
private:
	// Private Attributes
	glm::ivec2								m_renderSize = glm::ivec2(1);
	ECSSystemList							m_ecsSystems;
	std::vector<Graphics_Technique*>		m_gfxTechs;
	std::shared_ptr<Graphics_Framebuffers>	m_graphicsFBOS;
	VisualFX								m_visualFX;
	std::shared_ptr<CameraBuffer>			m_cameraBuffer;
	std::shared_ptr<RH_Volume>				m_volumeRH;
	std::shared_ptr<bool>					m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_MODULE_H