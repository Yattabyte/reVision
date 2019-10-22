#pragma once
#ifndef ROTATIONINDICATOR_H
#define ROTATIONINDICATOR_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


// Forward declarations
class Engine;

/** A level editor UI element displaying which direction the camera is rotated towards. */
class RotationIndicator final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy the rotation indicator. */
	~RotationIndicator();
	/** Construct a rotation indicator.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	RotationIndicator(Engine* engine);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	GLuint m_fboID, m_texID, m_depthID;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_3dIndicator;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirectIndicator;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // ROTATIONINDICATOR_H