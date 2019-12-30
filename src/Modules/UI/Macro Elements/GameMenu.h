#pragma once
#ifndef GAMEMENU_H
#define GAMEMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/Options_Levels.h"
#include "Modules/UI/FocusMap.h"


/** A UI element serving as an game menu. */
class GameMenu final : public Menu {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_levelSelect = (int)UI_Element::Interact::last_interact_index,
		on_back,
	};


	// Public (De)Constructors
	/** Construct an game menu.
	@param	engine		reference to the engine to use. */
	explicit GameMenu(Engine& engine);


	/** Get the selected level to open. */
	std::string getLevel() const;


protected:
	// Protected Methods
	/** Choose 'Level Select' from the options menu. */
	void levelSelect();
	/** Choose 'Back' from the options menu. */
	void back();


	// Protected Attributes
	std::shared_ptr<Options_Levels> m_mapMenu;
};

#endif // GAMEMENU_H