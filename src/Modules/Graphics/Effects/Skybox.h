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
	~Skybox();
	/** Construct a screen-space skybox effect.
	@param	engine			reference to the engine to use. */
	explicit Skybox(Engine& engine);


	// Public Interface Implementations.
	void clearCache(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) final;


private:
	// Private Methods
	/** Convolute the skybox cubemap, generating blurred MIPs (for rougher materials).
	@param	viewport	the viewport to render from. */
	void convoluteSky(const Viewport& viewport) noexcept;


	// Private but deleted
	/** Disallow default constructor. */
	inline Skybox() noexcept = delete;
	/** Disallow move constructor. */
	inline Skybox(Skybox&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline Skybox(const Skybox&) noexcept = delete;
	/** Disallow move assignment. */
	inline Skybox& operator =(Skybox&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline Skybox& operator =(const Skybox&) noexcept = delete;


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