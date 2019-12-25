#pragma once
#ifndef OPENDIALOGUE_H
#define OPENDIALOGUE_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Assets/Texture.h"
#include <string>
#include <vector>


/** A level editor UI element allowing the user to open levels in the maps folder. */
class OpenDialogue final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Construct a level opening dialogue.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	OpenDialogue(Engine& engine, LevelEditor_Module& editor);


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


private:
	// Private Methods
	/** Populate the level dialogue with an optional sub-directory.
	@param	directory	if non-blank, a sub-folder within the maps folder. */
	void populateLevels(const std::string& directory = "");
	/** Tick the main dialogue, rendering it and performing all logic. */
	void tickMainDialogue();
	/** Tick the rename dialogue, rendering it and performing all logic. */
	void tickRenameDialogue();
	/** Tick the delete dialogue, rendering it and performing all logic. */
	void tickDeleteDialogue();


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	std::string m_chosen = "", m_subDirectory = "";
	struct LevelEntry {
		std::string name = "", path = "", extension = "", extType = "", date = "", size = "";
		enum class Type {
			NONE,
			FILE,
			FOLDER,
			BACK
		} type = Type::NONE;
	};
	std::vector<LevelEntry> m_levels;
	int m_selected = -1;
	Shared_Texture m_iconFile, m_iconFolder, m_iconBack, m_iconRefresh;
};

#endif // OPENDIALOGUE_H