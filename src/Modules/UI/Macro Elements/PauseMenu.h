#pragma once
#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Engine.h"


/** A UI element serving as a pause menu. */
class PauseMenu : public Menu {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_resume_game = (int)UI_Element::Interact::last_interact_index,
		on_options,
		on_end,
	};


	// Public (De)Constructors
	/** Destroy the start menu. */
	inline ~PauseMenu() = default;
	/** Construct a start menu.
	@param	engine		reference to the engine to use. */
	explicit PauseMenu(Engine& engine) noexcept;


protected:
	// Protected Methods
	/** Choose 'resume' from the pause menu. */
	void resume() noexcept;
	/** Choose 'options' from the main menu. */
	void goToOptions() noexcept;
	/** Chosen when control is returned from the options menu. */
	void returnFromOptions() noexcept;
	/** Choose 'quit' from the pause menu. */
	void quit() noexcept;


	// Protected Attributes
	std::shared_ptr<OptionsMenu> m_optionsMenu;
};

#endif // PAUSEMENU_H