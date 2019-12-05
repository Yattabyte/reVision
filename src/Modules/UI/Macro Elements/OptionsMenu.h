#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/Options_Video.h"
#include "Modules/UI/Macro Elements/Options_Graphics.h"
#include "Modules/UI/FocusMap.h"
#include "Engine.h"


/** A UI element serving as an options menu. */
class OptionsMenu : public Menu {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_video = (int)UI_Element::Interact::last_interact_index,
		on_graphics,
		on_controls,
		on_back,
	};


	// Public (De)Constructors
	/** Destroy the options menu. */
	inline ~OptionsMenu() = default;
	/** Construct an options menu.
	@param	engine		reference to the engine to use. */
	explicit OptionsMenu(Engine& engine) noexcept;


protected:
	// Protected Methods
	/** Choose 'video' from the options menu. */
	void video() noexcept;
	/** Choose 'graphics' from the options menu. */
	void graphics() noexcept;
	/** Choose 'controls' from the options menu. */
	void controls() noexcept;
	/** Choose 'back' from the options menu. */
	void back() noexcept;


	// Protected Attributes
	std::shared_ptr<UI_Element> m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H