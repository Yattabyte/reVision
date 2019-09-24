#pragma once
#ifndef UNSAVEDCHANGESDIALOGUE_H
#define UNSAVEDCHANGESDIALOGUE_H

#include "Modules/UI/UI_M.h"
#include <string>
#include <vector>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element which prompts the user to save or ignore unsaved changes in a level. */
class UnsavedChangesDialogue final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this Unsaved Changes dialogue. */
	inline ~UnsavedChangesDialogue() = default;
	/** Construct a Unsaved Changes diaglogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	UnsavedChangesDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


	// Public Methods
	/** Enable the dialogue for opening a level. */
	void tryPrompt(const std::function<void()>& funcAfterPrompt);


private:
	// Private Methods
	/** Executes a stored function awaiting the level's changes to be saved, or ignored. */
	void executeFunction();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	std::function<void()> m_func;
};

#endif // UNSAVEDCHANGESDIALOGUE_H