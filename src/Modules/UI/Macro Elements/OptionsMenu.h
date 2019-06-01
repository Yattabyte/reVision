#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/Options_Video.h"
#include "Modules/UI/Macro Elements/Options_Graphics.h"


/** A UI element serving as an options menu. */
class OptionsMenu : public Menu {
public:
	// Public Interaction Enums
	const enum interact {
		on_video = last_interact_index,
		on_graphics,
		on_controls,
		on_back,
	};


	// Public (de)Constructors
	/** Destroy the options menu. */
	inline ~OptionsMenu() = default;
	/** Construct an options menu.
	@param	engine		the engine to use. */
	inline OptionsMenu(Engine * engine, UI_Element * parent = nullptr)
		: Menu(engine, parent) {
		// Title
		m_title->setText("OPTIONS");

		// Add 'Video' button
		m_videoMenu = std::make_shared<Options_Video>(engine, this);
		m_videoMenu->setVisible(false);
		addButton(engine, "VIDEO", [&]() { video(); });
		addElement(m_videoMenu);

		// Add 'Graphics' button
		m_gfxMenu = std::make_shared<Options_Graphics>(engine, this);
		m_gfxMenu->setVisible(false);
		addButton(engine, "GRAPHICS", [&]() { graphics(); });
		addElement(m_gfxMenu);

		// Add 'Controls' button
		addButton(engine, "CONTROLS", [&]() { controls(); });

		// Add 'Back' button
		addButton(engine, "< BACK  ", [&]() { back(); });

		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_videoMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
			m_gfxMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
			m_videoMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
			m_gfxMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		});
	}


	// Public Interface Implementations
	inline virtual void userAction(ActionState & actionState) override {
		if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS)
			back();
		else
			m_layout->userAction(actionState);
	}


protected:	
	// Protected Methods
	/** Choose 'video' from the options menu. */
	inline void video() {
		m_videoMenu->setVisible(true);
		m_gfxMenu->setVisible(false);
		// Transfer control only to the video menu
		m_engine->getModule_UI().setFocusedElement(m_videoMenu.get());
		enactCallback(on_video);
	}
	/** Choose 'graphics' from the options menu. */
	inline void graphics() {
		m_videoMenu->setVisible(false);
		m_gfxMenu->setVisible(true);
		// Transfer control only to the video menu
		m_engine->getModule_UI().setFocusedElement(m_gfxMenu.get());
		enactCallback(on_graphics);
	}
	/** Choose 'controls' from the options menu. */
	inline void controls() {
		enactCallback(on_controls);
	}
	/** Choose 'back' from the options menu. */
	inline void back() {
		m_videoMenu->setVisible(false);
		m_gfxMenu->setVisible(false);
		m_engine->getPreferenceState().save();

		// Revert appearance and control back to previous element (start menu, pause menu, etc)
		m_engine->getModule_UI().popRootElement();
		m_layout->setSelectionIndex(-1);
		enactCallback(on_back);
	}


	// Protected Attributes
	std::shared_ptr<UI_Element> m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H