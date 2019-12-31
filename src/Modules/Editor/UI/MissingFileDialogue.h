#pragma once
#ifndef MISSINGFILEDIALOGUE_H
#define MISSINGFILEDIALOGUE_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include <string>


/** A level editor UI element which notifies the user that a file they've requested is missing. */
class MissingFileDialogue final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Construct a missing file dialogue.
	@param	engine		reference to the engine to use.
	@param	editor		reference to the level-editor to use. */
	MissingFileDialogue(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


	// Public Methods
	/** Notify the user that a file is missing.
	@param	filename	the file name of the missing file. */
	void notifyMissing(const std::string& filename);


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	std::string m_fileName;
};

#endif // MISSINGFILEDIALOGUE_H