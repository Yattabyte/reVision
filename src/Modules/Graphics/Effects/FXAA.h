#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A post-processing technique for applying FXAA to the currently bound 2D image. */
class FXAA final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~FXAA();
	/** Construct a FXAA effect.
	@param	engine			reference to the engine to use. */
	explicit FXAA(Engine& engine);


	// Public Interface Implementations.
	void clearCache(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) final;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline FXAA() noexcept = delete;
	/** Disallow move constructor. */
	inline FXAA(FXAA&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline FXAA(const FXAA&) noexcept = delete;
	/** Disallow move assignment. */
	inline FXAA& operator =(FXAA&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline FXAA operator =(const FXAA&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shaderFXAA;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FXAA_H