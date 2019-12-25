#pragma once
#ifndef SSR_H
#define SSR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A core-rendering technique for deriving extra reflection information from the viewport itself. */
class SSR final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this Effect. */
	~SSR();
	/** Construct a screen-space reflection effect.
	@param	engine			reference to the engine to use. */
	explicit SSR(Engine& engine);


	// Public Interface Implementations.
	void clearCache(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) final;


private:
	// Private Methods
	/** Convolute the lighting buffer into each of its MIP levels.
	@param	viewport	the viewport to render from. */
	void updateMIPChain(Viewport& viewport);


	// Private but deleted
	/** Disallow default constructor. */
	inline SSR() noexcept = delete;
	/** Disallow move constructor. */
	inline SSR(SSR&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline SSR(const SSR&) noexcept = delete;
	/** Disallow move assignment. */
	inline SSR& operator =(SSR&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline SSR& operator =(const SSR&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shaderSSR1, m_shaderSSR2, m_shaderCopy, m_shaderConvMips;
	Shared_Auto_Model m_shapeQuad;
	GLuint m_bayerID = 0;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SSR_H