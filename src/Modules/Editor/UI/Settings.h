#pragma once
#ifndef SETTINGS_H
#define SETTINGS_H

#include "Modules/Editor/UI/Editor_Interface.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor dialogue allowing the user to modify level editor settings. */
class Settings final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this settings dialogue. */
	inline ~Settings() = default;
	/** Construct a settings dialogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	Settings(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


private:
	// Private Methods
	/** Tick the main dialogue, rendering it and performing all logic. */
	void tickMainDialogue();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // SETTINGS_H