#pragma once
#ifndef ROTATIONINDICATOR_H
#define ROTATIONINDICATOR_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


/** A level editor UI element displaying which direction the camera is rotated towards. */
class RotationIndicator final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy the rotation indicator. */
	~RotationIndicator();
	/** Construct a rotation indicator.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	explicit RotationIndicator(Engine& engine);


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline RotationIndicator() noexcept = delete;
	/** Disallow move constructor. */
	inline RotationIndicator(RotationIndicator&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline RotationIndicator(const RotationIndicator&) noexcept = delete;
	/** Disallow move assignment. */
	inline RotationIndicator& operator =(RotationIndicator&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline RotationIndicator& operator =(const RotationIndicator&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	GLuint m_fboID = 0, m_texID = 0, m_depthID = 0;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_3dIndicator;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirectIndicator;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // ROTATIONINDICATOR_H