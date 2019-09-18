#pragma once
#ifndef SETTINGSDIALOGUE_H
#define SETTINGSDIALOGUE_H

#include "Modules/UI/UI_M.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor dialogue allowing the user to modify level editor settings. */
class SettingsDialogue : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this settings dialogue. */
	inline ~SettingsDialogue() = default;
	/** Construct a settings diaglogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	SettingsDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


private:
	// Private Methods
	/** Tick the main dialogue, rendering it and performing all logic. */
	void tickMainDialogue();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // SETTINGSDIALOGUE_H