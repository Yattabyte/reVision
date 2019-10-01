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

/** A top-level UI element representing the entire level editor's GUI. */
class Editor_Interface final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy the level editor UI. */
	inline ~Editor_Interface() = default;
	/** Creates the level editor UI.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Editor_Interface(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


	// Public Attributes
	std::shared_ptr<ImGUI_Element>
		m_uiCamController,
		m_uiRotIndicator,
		m_uiTitlebar,
		m_uiPrefabs,
		m_uiSceneInspector,
		m_uiEntityInspector,
		m_uiSettings,
		m_uiRecoverDialogue,
		m_uiOpenDialogue,
		m_uiSaveDialogue,
		m_uiUnsavedDialogue,
		m_uiMissingDialogue;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	Shared_Auto_Model m_shapeQuad;
	Shared_Shader m_shader;
	IndirectDraw m_indirectQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // EDITOR_INTERFACE_H