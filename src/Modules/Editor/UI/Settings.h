#pragma once
#ifndef SETTINGS_H
#define SETTINGS_H

#include "Modules/Editor/UI/Editor_Interface.h"


/** A level editor dialogue allowing the user to modify level editor settings. */
class Settings final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this settings dialogue. */
	inline ~Settings() = default;
	/** Construct a settings dialogue.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Settings(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


private:
	// Private Methods
	/** Tick the main dialogue, rendering it and performing all logic. */
	void tickMainDialogue();


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // SETTINGS_H