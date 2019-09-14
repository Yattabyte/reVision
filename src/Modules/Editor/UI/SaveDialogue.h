#pragma once
#ifndef SAVEDIALOGUE_H
#define SAVEDIALOGUE_H

#include "Modules/UI/UI_M.h"
#include "Assets/Texture.h"
#include <string>
#include <vector>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element allowing the user to save levels in the maps folder. */
class SaveDialogue : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this save dialogue. */
	inline ~SaveDialogue() = default;
	/** Construct a save diaglogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	SaveDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


	// Public Methods
	/** Trigger the dialogue for saving a level. */
	void startDialogue();


private:
	// Private Methods
	/** Populate the level dialogue with an optional sub-directory.
	@param	directory	if non-blank, a subfolder within the maps folder. */
	void populateLevels(const std::string& directory = "");
	/***/
	void tickMainDialogue();
	/***/
	void tickOverwriteDialogue();
	/***/
	void tickRenameDialogue();
	/***/
	void tickDeleteDialogue();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	bool m_popupOpen = false, m_paused = false;
	std::string m_chosen = "", m_subDirectory = "";
	struct LevelEntry {
		std::string name = "", path = "", extension = "", extType = "", date = "", size = "";
		enum type {
			none,
			file,
			folder,
			back
		} type = none;
	};
	std::vector<LevelEntry> m_levels;
	int m_selected = -1;
	Shared_Texture m_iconFile, m_iconFolder, m_iconBack, m_iconRefresh;
};

#endif // SAVEDIALOGUE_H