#pragma once
#ifndef HDR_H
#define HDR_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A post-processing technique for tone-mapping and gamma correcting the final lighting product. */
class HDR final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~HDR();
	/** Construct a HDR effect.
	@param	engine			reference to the engine to use. */
	explicit HDR(Engine& engine);


	// Public Interface Implementations.
	void clearCache(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) final;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline HDR() noexcept = delete;
	/** Disallow move constructor. */
	inline HDR(HDR&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline HDR(const HDR&) noexcept = delete;
	/** Disallow move assignment. */
	inline HDR& operator =(HDR&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline HDR& operator =(const HDR&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	float m_gamma = 1.0f;
	Shared_Shader m_shaderHDR;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // HDR_H