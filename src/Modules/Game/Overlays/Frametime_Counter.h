#pragma once
#ifndef FRAMETIME_COUNTER_H
#define FRAMETIME_COUNTER_H

#include "Modules/Game/Overlays/Overlay.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "glm/glm.hpp"


/** A post-processing technique for writing the frame time to the screen. */
class Frametime_Counter final : public Overlay {
public:
	// Public (De)Constructors
	/** Destroy this overlay. */
	~Frametime_Counter() noexcept;
	/** Construct a frame-time counter.
	@param	engine		reference to the engine to use. */
	explicit Frametime_Counter(Engine& engine) noexcept;


	// Public Interface Implementations.
	virtual void applyEffect(const float& deltaTime) noexcept override final;


private:
	// Private Methods
	/** Resize this indicator.
	@param	size		the new size to use. */
	void resize(const glm::ivec2& size) noexcept;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shader;
	Shared_Texture m_numberTexture;
	Shared_Auto_Model m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_projMatrix = glm::mat4(1.0f);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // FRAMETIME_COUNTER_H