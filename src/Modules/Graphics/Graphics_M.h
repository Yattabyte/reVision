#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/MappedChar.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"


/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this graphics rendering module. */
	inline ~Graphics_Module() = default;
	/** Construct a graphics rendering module. */
	inline Graphics_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Render using our graphics pipeline, from the camera buffer specified into the framebuffers and volume specified.
	@param	deltaTime		the amount of time since last frame.
	@param	viewport		the view port to render into.
	@param	categories		the technique categories to allow for rendering, defaults to ALL. */
	void renderScene(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const CameraBuffer::CamStruct * camera, const unsigned int & allowedCategories = Graphics_Technique::ALL);
	/***/
	void cullShadows(const float & deltaTime, const std::vector<std::pair<CameraBuffer::CamStruct*, int>> & perspectives);	
	/***/
	void renderShadows(const float & deltaTime);
	/***/
	void genPerspectiveMatrix();
	/** Returns a shared pointer to the primary camera.
	@return					the primary camera. */
	inline std::shared_ptr<CameraBuffer> getClientCamera() const {
		return m_clientCamera;
	}

	
private:
	// Private Methods
	/***/
	void copyToScreen();


	// Private Attributes
	glm::ivec2									m_renderSize = glm::ivec2(1);
	ECSSystemList								m_systems;
	std::unique_ptr<Graphics_Pipeline>			m_pipeline;
	std::shared_ptr<Viewport>					m_viewport;
	std::shared_ptr<CameraBuffer>				m_clientCamera;
	Shared_Shader								m_shader;
	Shared_Primitive							m_shapeQuad;
	StaticBuffer								m_quadIndirectBuffer;
	std::shared_ptr<bool>						m_aliveIndicator = std::make_shared<bool>(true);
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>>	m_sceneCameras;
};

#endif // GRAPHICS_MODULE_H