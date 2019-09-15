#pragma once
#ifndef RECOVERDIALOGUE_H
#define RECOVERDIALOGUE_H

#include "Modules/UI/UI_M.h"
#include <filesystem>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element prompting the user to recover an auto-saved file. */
class RecoverDialogue : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this level dialogue. */
	inline ~RecoverDialogue() = default;
	/** Construct a level diaglogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	RecoverDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


	// Public Methods
	/** Trigger the dialogue for opening a level. */
	void startDialogue(const std::filesystem::path& path);


private:
	// Private Methods
	/***/
	void tickMainDialogue();
	/***/
	void tickDeleteDialogue();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	bool m_popupOpen = false;
	std::filesystem::path m_recoveredPath;
};

#endif // RECOVERDIALOGUE_H