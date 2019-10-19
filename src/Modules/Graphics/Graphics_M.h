#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/ECS/ecsWorld.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/IndirectDraw.h"


// Forward Declarations
class Camera;
class RH_Volume;
struct Viewport;

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
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;


	// Public Methods
	/***/
	inline auto getPipeline() {
		return m_pipeline;
	}
	/***/
	void renderWorld(ecsWorld& world, const float& deltaTime, const GLuint& fboID = 0);
	/***/
	void renderWorld(ecsWorld& world, const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::shared_ptr<Camera>>& cameras);
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
	void copyToScreen(const GLuint& fboID);


	// Private Attributes
	glm::ivec2										m_renderSize = glm::ivec2(1);
	std::shared_ptr<Graphics_Pipeline>				m_pipeline;
	std::shared_ptr<Viewport>						m_viewport;
	std::shared_ptr<Camera>							m_clientCamera;
	std::shared_ptr<RH_Volume>						m_rhVolume;
	Shared_Shader									m_shader;
	Shared_Auto_Model								m_shapeQuad;
	IndirectDraw									m_indirectQuad;
	std::shared_ptr<bool>							m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_MODULE_H