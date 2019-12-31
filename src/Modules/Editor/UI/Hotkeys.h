#pragma once
#ifndef HOTKEYS_H
#define HOTKEYS_H

#include "Modules/Editor/UI/Editor_Interface.h"


/** An invisible level editor UI element responsible for handling hotkeys. */
class Hotkeys final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Construct this element.
	@param	engine		reference to the engine to use.
	@param	editor		reference to the level-editor to use. */
	Hotkeys(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // HOTKEYS_H