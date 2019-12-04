#pragma once
#ifndef HOTKEYS_H
#define HOTKEYS_H

#include "Modules/Editor/UI/Editor_Interface.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** An invisible level editor UI element responsible for handling hotkeys. */
class Hotkeys final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this element. */
	inline ~Hotkeys() = default;
	/** Construct this element.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Hotkeys(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // HOTKEYS_H
