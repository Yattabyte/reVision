#pragma once
#ifndef EDITOR_INTERFACE_H
#define EDITOR_INTERFACE_H

#include "Modules/UI/UI_M.h"
#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Utilities/GL/IndirectDraw.h"


// Forward Declarations
class Engine;
class LevelEditor_Module;
class CameraController;
class TitleBar;
class Prefabs;
class Inspector;
class RotationIndicator;
class LevelDialogue;

/***/
class Editor_Interface : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy the editor. */
	inline ~Editor_Interface() = default;
	/** Creates an editor. */
	Editor_Interface(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


	// Public Attributes
	std::shared_ptr<CameraController> m_uiCamController;
	std::shared_ptr<TitleBar> m_uiTitlebar;
	std::shared_ptr<Prefabs> m_uiPrefabs;
	std::shared_ptr<Inspector> m_uiInspector;
	std::shared_ptr<RotationIndicator> m_uiRotIndicator;
	std::shared_ptr<LevelDialogue> m_uiLevelDialogue;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
	Shared_Auto_Model m_shapeQuad;
	Shared_Shader m_shader;
	IndirectDraw m_indirectQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // EDITOR_INTERFACE_H