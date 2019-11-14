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
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Hotkeys(Engine* engine, LevelEditor_Module* editor) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // HOTKEYS_H