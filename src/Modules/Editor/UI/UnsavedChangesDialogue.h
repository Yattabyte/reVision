#pragma once
#ifndef UNSAVEDCHANGESDIALOGUE_H
#define UNSAVEDCHANGESDIALOGUE_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include <functional>
#include <string>
#include <vector>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element which prompts the user to save or ignore unsaved changes in a level. */
class UnsavedChangesDialogue final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this Unsaved Changes dialogue. */
	inline ~UnsavedChangesDialogue() = default;
	/** Construct a Unsaved Changes dialogue.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	UnsavedChangesDialogue(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


	// Public Methods
	/** Checks if the editor has unsaved changes, otherwise executes the supplied function. Prompts user to save. 
	@param	funcAfterPrompt		the function to call if the editor is safe to do so. */
	void tryPrompt(const std::function<void()>& funcAfterPrompt) noexcept;


private:
	// Private Methods
	/** Executes a stored function awaiting the level's changes to be saved, or ignored. */
	void executeFunction() noexcept;


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	std::function<void()> m_func;
};

#endif // UNSAVEDCHANGESDIALOGUE_H
