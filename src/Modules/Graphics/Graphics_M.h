#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/ECS/ecsWorld.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/IndirectDraw.h"


// Forward Declarations
class Camera;
struct Viewport;

/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this graphics rendering module. */
	inline ~Graphics_Module() = default;
	/** Construct a graphics rendering module.
	@param	engine		reference to the engine to use. */
	explicit Graphics_Module(Engine& engine);


	// Public Interface Implementation
	virtual void initialize() noexcept override final;
	virtual void deinitialize() noexcept override final;


	// Public Methods
	/** Retrieve a shared pointer to the rendering pipeline.
	@return					shared pointer to the rendering pipeline. */
	std::shared_ptr<Graphics_Pipeline> getPipeline() const noexcept;
	/** Convenience function for rendering a given ecsWorld to a given FBO.
	@param	world			the ecsWorld to source data from.
	@param	deltaTime		the amount of time passed since last frame.
	@param	fboID			the FBO to render to. */
	void renderWorld(ecsWorld& world, const float& deltaTime, const GLuint& fboID = 0) noexcept;
	/** Convenience function for rendering a given ecsWorld into the pipeline.
	@param	world			the ecsWorld to source data from.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render into.
	@param	cameras			the cameras to render from. */
	void renderWorld(ecsWorld& world, const float& deltaTime, const std::shared_ptr<Viewport>& viewport, std::vector<Camera>& cameras) noexcept;
	/** Generates a perspective matrix for the client camera. */
	void genPerspectiveMatrix() noexcept;
	/** Retrieves a shared pointer to the primary camera.
	@return					the primary camera. */
	Camera& getClientCamera() noexcept;


private:
	// Private Methods
	/** Copy the client camera's final color buffer to a specific framebuffer.
	@param	fboID			the framebuffer ID. */
	void copyToFramebuffer(const GLuint& fboID) noexcept;


	// Private Attributes
	glm::ivec2										m_renderSize = glm::ivec2(1);
	std::shared_ptr<Graphics_Pipeline>				m_pipeline;
	std::shared_ptr<Viewport>						m_viewport;
	Camera											m_clientCamera;
	Shared_Shader									m_shader;
	Shared_Auto_Model								m_shapeQuad;
	IndirectDraw<1>									m_indirectQuad;
	std::shared_ptr<bool>							m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_MODULE_H