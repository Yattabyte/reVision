#pragma once
#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"


/** A UI element serving as a pause menu. */
class PauseMenu final : public Menu {
public:
	// Public Interaction Enums
	/** Enumerations for interacting with this element. */
	enum class Interact : int {
		on_resume_game = (int)UI_Element::Interact::last_interact_index,
		on_options,
		on_end,
	};


	// Public (De)Constructors
	/** Construct a start menu.
	@param	engine		reference to the engine to use. */
	explicit PauseMenu(Engine& engine);


protected:
	// Protected Methods
	/** Choose 'resume' from the pause menu. */
	void resume();
	/** Choose 'options' from the main menu. */
	void goToOptions();
	/** Chosen when control is returned from the options menu. */
	void returnFromOptions() noexcept;
	/** Choose 'quit' from the pause menu. */
	void quit();


	// Protected Attributes
	std::shared_ptr<OptionsMenu> m_optionsMenu;
};

#endif // PAUSEMENU_H