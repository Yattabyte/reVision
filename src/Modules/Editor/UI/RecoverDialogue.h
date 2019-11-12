#pragma once
#ifndef RECOVERDIALOGUE_H
#define RECOVERDIALOGUE_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include <filesystem>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element prompting the user to recover an auto-saved file. */
class RecoverDialogue final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this level dialogue. */
	inline ~RecoverDialogue() = default;
	/** Construct a level dialogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	RecoverDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


	// Public Methods
	/** Set the recovered file path.
	@param	path		the recovered file path*/
	void setPath(const std::filesystem::path& path);


private:
	// Private Methods
	/** Tick the main dialogue, rendering it and performing all logic. */
	void tickMainDialogue();
	/** Tick the delete dialogue, rendering it and performing all logic. */
	void tickDeleteDialogue();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	std::filesystem::path m_recoveredPath;
};

#endif // RECOVERDIALOGUE_H