#pragma once
#ifndef MISSINGFILEDIALOGUE_H
#define MISSINGFILEDIALOGUE_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include <string>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element which notifies the user that a file they've requested is missing. */
class MissingFileDialogue final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this missing file dialogue. */
	inline ~MissingFileDialogue() = default;
	/** Construct a missing file dialogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	MissingFileDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


	// Public Methods
	/** Notify the user that a file is missing. */
	void notifyMissing(const std::string& filename);


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	std::string m_fileName;
};

#endif // MISSINGFILEDIALOGUE_H