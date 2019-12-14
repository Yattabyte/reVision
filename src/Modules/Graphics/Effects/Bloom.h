#pragma once
#ifndef BLOOM_H
#define BLOOM_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A post-processing technique for generating bloom from a lighting buffer. */
class Bloom final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~Bloom() noexcept;
	/** Construct a bloom effect.
	@param	engine			reference to the engine to use. */
	explicit Bloom(Engine& engine) noexcept;


	// Public Interface Implementations.
	void clearCache(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept final;


private:
	// Private Methods
	/** Change the strength of the bloom effect.
	@param	strength		the new strength of the bloom effect. */
	void setBloomStrength(const int& strength) noexcept;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shaderBloomExtract, m_shaderCopy, m_shaderGB;
	Shared_Auto_Model m_shapeQuad;
	int m_bloomStrength = 5;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // BLOOM_H