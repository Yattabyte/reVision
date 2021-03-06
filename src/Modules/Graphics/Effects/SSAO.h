#pragma once
#ifndef SSAO_H
#define SSAO_H
#define MAX_KERNEL_SIZE 128

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A core-rendering technique for deriving an ambient occlusion factor from the viewport itself. */
class SSAO final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~SSAO();
	/** Construct a screen-space ambient occlusion effect.
	@param	engine			reference to the engine to use. */
	explicit SSAO(Engine& engine);


	// Public Interface Implementations.
	void clearCache(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) final;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline SSAO() noexcept = delete;
	/** Disallow move constructor. */
	inline SSAO(SSAO&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline SSAO(const SSAO&) noexcept = delete;
	/** Disallow move assignment. */
	inline SSAO& operator =(SSAO&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline SSAO& operator =(const SSAO&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shader, m_shaderCopyAO, m_shaderGB_A;
	Shared_Auto_Model m_shapeQuad;
	float m_radius = 1.0f;
	int m_quality = 1, m_blurStrength = 5;
	GLuint m_noiseID = 0;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSAO_H