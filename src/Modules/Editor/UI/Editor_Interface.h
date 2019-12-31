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
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Editor_Interface(Engine& engine, LevelEditor_Module& editor);


	// Public Methods
	/** Tick all of this interface's elements and render them. 
	@param	deltaTime	the amount of time passed since last frame. */
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
	// Private but deleted
	/** Disallow default constructor. */
	inline Editor_Interface() noexcept = delete;
	/** Disallow move constructor. */
	inline Editor_Interface(Editor_Interface&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline Editor_Interface(const Editor_Interface&) noexcept = delete;
	/** Disallow move assignment. */
	inline Editor_Interface& operator =(Editor_Interface&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline Editor_Interface& operator =(const Editor_Interface&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};


/** UI element representing an ImGUI element. */
struct ImGUI_Element {
	// Public (De)Constructors
	/** Virtual Destructor. */
	inline virtual ~ImGUI_Element() = default;
	/** Default constructor. */
	inline ImGUI_Element() noexcept = default;
	/** Move constructor. */
	inline ImGUI_Element(ImGUI_Element&&) noexcept = default;
	/** Copy constructor. */
	inline ImGUI_Element(const ImGUI_Element&) noexcept = default;

	
	// Public Methods
	/** Move assignment. */
	inline ImGUI_Element& operator =(ImGUI_Element&&) noexcept = default;
	/** Copy assignment. */
	inline ImGUI_Element& operator =(const ImGUI_Element&) noexcept = default;
	/** Open this element. */
	void open() noexcept;
	/** Close this element. */
	void close() noexcept;

	// Public Interface Declaration
	/** Tick this element, updating it and rendering it. 
	@param	deltaTime		the amount of time since last frame. */
	virtual void tick(const float& deltaTime);


	// Public Attributes
	bool m_open = true;
};

#endif // EDITOR_INTERFACE_H