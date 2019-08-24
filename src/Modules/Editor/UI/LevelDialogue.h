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
class LevelEditor_Module;

/** Allows the user to control the camera whenever not interacting with other UI elements. */
class LevelDialogue : public ImGUI_Element {
public:
	// Public (de)Constructors
	/***/
	inline ~LevelDialogue() = default;
	/***/
	LevelDialogue(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override;


	// Public Methods
	/***/
	void startOpenDialogue();
	/***/
	void startSaveDialogue();


private:
	// Private Methods
	/***/
	void populateLevels(const std::string& directory = "");


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
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