#pragma once
#ifndef ROTATIONINDICATOR_H
#define ROTATIONINDICATOR_H

#include "Modules/UI/UI_M.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** Allows the user to control the camera whenever not interacting with other UI elements. */
class RotationIndicator : public ImGUI_Element {
public:
	// Public (de)Constructors
	/***/
	~RotationIndicator();
	/***/
	RotationIndicator(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
	GLuint m_fboID, m_texID, m_depthID;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_3dIndicator;
	Shared_Shader m_gizmoShader;
	StaticBuffer m_indicatorIndirectBuffer;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // ROTATIONINDICATOR_H