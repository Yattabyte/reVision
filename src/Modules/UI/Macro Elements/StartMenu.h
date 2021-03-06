#pragma once
#ifndef STARTMENU_H
#define STARTMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/GameMenu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"


/** A UI element serving as a start menu. */
class StartMenu final : public Menu {
public:
	// Public Interaction Enums
	/** Enumerations for interacting with this element. */
	enum class Interact : int {
		on_start_game = (int)UI_Element::Interact::last_interact_index,
		on_level_editor,
		on_options,
		on_quit,
	};


	// Public (De)Constructors
	/** Construct a start menu.
	@param	engine		reference to the engine to use. */
	explicit StartMenu(Engine& engine);


protected:
	// Protected Methods
	/** Choose 'start game' from the main menu. */
	void startGame();
	/** Choose 'level editor' from the main menu. */
	void startEditor();
	/** Choose 'options' from the main menu. */
	void goToOptions();
	/** Choose 'quit' from the main menu. */
	void quit();


	// Protected Attributes
	std::shared_ptr<GameMenu> m_gameMenu;
	std::shared_ptr<OptionsMenu> m_optionsMenu;
};

#endif // STARTMENU_H