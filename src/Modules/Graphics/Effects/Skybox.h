#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Cubemap.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A core-rendering technique for writing the frame time to the screen. */
class Skybox final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~Skybox() noexcept;
	/** Construct a screen-space skybox effect.
	@param	engine			reference to the engine to use. */
	explicit Skybox(Engine& engine) noexcept;


	// Public Interface Implementations.
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;


private:
	// Private Methods
	/** Convolute the skybox cubemap, generating blurred MIPs (for rougher materials).
	@param	viewport	the viewport to render from. */
	void convoluteSky(const std::shared_ptr<Viewport>& viewport) noexcept;


	// Private Attributes
	Engine& m_engine;
	GLuint m_cubeFBO = 0, m_cubemapMipped = 0;
	Shared_Cubemap m_cubemapSky;
	Shared_Shader m_shaderSky, m_shaderSkyReflect, m_shaderConvolute;
	Shared_Auto_Model m_shapeQuad;
	bool m_skyOutOfDate = false;
	glm::ivec2 m_skySize = glm::ivec2(1);
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT), quad6IndirectBuffer = IndirectDraw((GLuint)6, 6, 0, GL_CLIENT_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FRAMETIME_COUNTER_H