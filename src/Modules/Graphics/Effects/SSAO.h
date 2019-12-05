#pragma once
#ifndef SSAO_H
#define SSAO_H
#define MAX_KERNEL_SIZE 128

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Engine.h"
#include <random>


/** A core-rendering technique for deriving an ambient occlusion factor from the viewport itself. */
class SSAO final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~SSAO() noexcept;
	/** Construct a screen-space ambient occlusion effect.
	@param	engine			reference to the engine to use. */
	explicit SSAO(Engine& engine) noexcept;


	// Public Interface Implementations.
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;


private:
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