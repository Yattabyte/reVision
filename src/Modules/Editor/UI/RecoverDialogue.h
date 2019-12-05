#pragma once
#ifndef RECOVERDIALOGUE_H
#define RECOVERDIALOGUE_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include <filesystem>


/** A level editor UI element prompting the user to recover an auto-saved file. */
class RecoverDialogue final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this level dialogue. */
	inline ~RecoverDialogue() = default;
	/** Construct a level dialogue.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	RecoverDialogue(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


	// Public Methods
	/** Set the recovered file path.
	@param	path		the recovered file path*/
	void setPath(const std::filesystem::path& path) noexcept;


private:
	// Private Methods
	/** Tick the main dialogue, rendering it and performing all logic. */
	void tickMainDialogue() noexcept;
	/** Tick the delete dialogue, rendering it and performing all logic. */
	void tickDeleteDialogue() noexcept;


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	std::filesystem::path m_recoveredPath;
};

#endif // RECOVERDIALOGUE_H