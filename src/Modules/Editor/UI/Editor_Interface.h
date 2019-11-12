#pragma once
#ifndef EDITOR_INTERFACE_H
#define EDITOR_INTERFACE_H

#include "glm/glm.hpp"
#include <memory>


// Forward Declarations
class Engine;
class LevelEditor_Module;
struct ImGUI_Element;

/** A top-level UI element representing the entire level editor's GUI. */
class Editor_Interface {
public:
	// Public (De)Constructors
	/** Destroy the level editor UI. */
	~Editor_Interface();
	/** Creates the level editor UI.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Editor_Interface(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	void tick(const float& deltaTime);


	// Public Attributes
	std::shared_ptr<ImGUI_Element>
		m_uiHotkeys,
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
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};


/** UI element representing an ImGUI element. */
struct ImGUI_Element {
	inline virtual ~ImGUI_Element() = default;
	inline ImGUI_Element() = default;
	bool m_open = true;
	inline void open() { m_open = true; }
	inline void close() { m_open = false; }
	inline virtual void tick(const float& deltaTime) {};
};

#endif // EDITOR_INTERFACE_H