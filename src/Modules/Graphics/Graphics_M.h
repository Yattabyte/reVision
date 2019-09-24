#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/MappedChar.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Utilities/GL/GL_ArrayBuffer.h"


/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module final : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this graphics rendering module. */
	inline ~Graphics_Module() = default;
	/** Construct a graphics rendering module. */
	inline Graphics_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override final;
	virtual void deinitialize() override final;
	virtual void frameTick(const float & deltaTime) override final;


	// Public Methods
	/** Render using our graphics pipeline, from the camera buffer specified into the framebuffers and volume specified.
	@param	deltaTime		the amount of time since last frame.
	@param	viewport		the view port to render into.
	@param	categories		the technique categories to allow for rendering, defaults to ALL. */
	void renderScene(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives, const unsigned int & allowedCategories = Graphics_Technique::ALL);
	/** Use geometry techniques to cull shadows.
	@param	deltaTime		the amount of time passed since last frame.
	@param	perspectives	the camera and layer indicies to render. */
	void cullShadows(const float & deltaTime, const std::vector<std::pair<int, int>> & perspectives);	
	/** Use geometry techniques to render shadows.
	@param	deltaTime		the amount of time passed since last frame. */
	void renderShadows(const float & deltaTime);
	/** Generates a perspective matrix for the client camera. */
	void genPerspectiveMatrix();
	/** Returns a shared pointer to the primary camera.
	@return					the primary camera. */
	inline std::shared_ptr<Camera> getClientCamera() const {
		return m_clientCamera;
	}

	
private:
	// Private Methods
	/** Copy the client camera's final color buffer to the screen. */
	void copyToScreen();


	// Private Attributes
	glm::ivec2										m_renderSize = glm::ivec2(1);
	ecsSystemList									m_systems;
	std::unique_ptr<Graphics_Pipeline>				m_pipeline;
	std::shared_ptr<Viewport>						m_viewport;
	std::shared_ptr<Camera>							m_clientCamera;
	std::shared_ptr<RH_Volume>						m_rhVolume;
	Shared_Shader									m_shader;
	Shared_Auto_Model								m_shapeQuad;
	IndirectDraw									m_indirectQuad;
	std::shared_ptr<bool>							m_aliveIndicator = std::make_shared<bool>(true);
	std::shared_ptr<std::vector<Camera*>>			m_sceneCameras;
	std::shared_ptr<GL_ArrayBuffer<Camera::GPUData>> m_cameraBuffer;
};

#endif // GRAPHICS_MODULE_H