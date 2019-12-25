#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/Options_Video.h"
#include "Modules/UI/Macro Elements/Options_Graphics.h"
#include "Modules/UI/FocusMap.h"


/** A UI element serving as an options menu. */
class OptionsMenu final : public Menu {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_video = (int)UI_Element::Interact::last_interact_index,
		on_graphics,
		on_controls,
		on_back,
	};


	// Public (De)Constructors
	/** Construct an options menu.
	@param	engine		reference to the engine to use. */
	explicit OptionsMenu(Engine& engine);


protected:
	// Protected Methods
	/** Choose 'video' from the options menu. */
	void video();
	/** Choose 'graphics' from the options menu. */
	void graphics();
	/** Choose 'controls' from the options menu. */
	void controls();
	/** Choose 'back' from the options menu. */
	void back();


	// Protected Attributes
	std::shared_ptr<UI_Element> m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H