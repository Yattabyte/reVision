#pragma once
#ifndef LEVELDIALOGUE_H
#define LEVELDIALOGUE_H

#include "Modules/UI/UI_M.h"
#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"


// Forward declarations
class Engine;

/** A level editor UI element allowing the user to view, open, and save levels in the maps folder. */
class LevelDialogue : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this level dialogue. */
	inline ~LevelDialogue() = default;
	/** Construct a level diaglogue.
	@param	engine		the currently active engine. */
	LevelDialogue(Engine* engine);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


	// Public Methods
	/** Enable the dialogue for opening a level. */
	void startOpenDialogue();
	/** Enable the dialogue for saving a level. */
	void startSaveDialogue();


private:
	// Private Methods
	/** Populate the level dialogue with an optional sub-directory.
	@param	directory	if non-blank, a subfolder within the maps folder. */
	void populateLevels(const std::string& directory = "");


	// Private Attributes
	Engine* m_engine = nullptr;
	bool m_popupOpen = false;
	bool m_openOrSave = true;
	std::string m_subDirectory;
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

#endif // LEVELDIALOGUE_H