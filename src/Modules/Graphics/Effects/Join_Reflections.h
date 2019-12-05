#pragma once
#ifndef JOIN_REFLECTIONS_H
#define JOIN_REFLECTIONS_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


/** A core-rendering technique for writing the final scene reflections back into the lighting. */
class Join_Reflections final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this effect. */
	~Join_Reflections() noexcept;
	/** Construct a reflection joining effect.
	@param	engine			reference to the engine to use. */
	explicit Join_Reflections(Engine& engine) noexcept;


	// Public Interface Implementations.
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shader;
	Shared_Texture m_brdfMap;
	Shared_Auto_Model m_shapeQuad;
	struct DrawData {
		DynamicBuffer<> camBufferIndex;
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	std::vector<DrawData> m_drawData;
	int	m_drawIndex = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // JOIN_REFLECTIONS_H