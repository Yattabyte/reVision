#pragma once
#ifndef ROTATIONINDICATOR_H
#define ROTATIONINDICATOR_H

#include "Modules/UI/UI_M.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element displaying which direction the camera is rotated towards. */
class RotationIndicator : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy the rotation indicator. */
	~RotationIndicator();
	/** Construct a rotation indicator. 
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	RotationIndicator(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	GLuint m_fboID, m_texID, m_depthID;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_3dIndicator;
	Shared_Shader m_gizmoShader;
	IndirectDraw m_indirectIndicator;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // ROTATIONINDICATOR_H