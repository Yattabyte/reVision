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
	~HDR() noexcept;
	/** Construct a HDR effect.
	@param	engine			reference to the engine to use. */
	explicit HDR(Engine& engine) noexcept;


	// Public Interface Implementations.
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;


private:
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