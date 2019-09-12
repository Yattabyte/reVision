#pragma once
#ifndef OPENDIALOGUE_H
#define OPENDIALOGUE_H

#include "Modules/UI/UI_M.h"
#include <string>
#include <vector>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element allowing the user to open levels in the maps folder. */
class OpenDialogue : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this level dialogue. */
	inline ~OpenDialogue() = default;
	/** Construct a level diaglogue.
	@param	engine		the currently active engine.
	@param	editor		the currently active level editor. */
	OpenDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


	// Public Methods
	/** Trigger the dialogue for opening a level. */
	void startDialogue();


private:
	// Private Methods
	/** Populate the level dialogue with an optional sub-directory.
	@param	directory	if non-blank, a subfolder within the maps folder. */
	void populateLevels(const std::string& directory = "");
	/***/
	void tickMainDialogue();
	/***/
	void tickDeleteDialogue();
	/***/
	void tickRenameDialogue();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	bool m_popupOpen = false;
	std::string m_chosen = "", m_subDirectory = "";
	struct LevelEntry {
		std::string name = "", path = "";
		enum type {
			none,
			file,
			folder,
			back
		} type = none;
	};
	std::vector<LevelEntry> m_levels;
	int m_selected = -1;
};

#endif // OPENDIALOGUE_H