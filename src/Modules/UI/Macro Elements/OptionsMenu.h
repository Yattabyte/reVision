#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Macro Elements/Menu.h"
#include "Modules/UI/Macro Elements/Options_Video.h"
#include "Modules/UI/Macro Elements/Options_Graphics.h"
#include "Engine.h"


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
	inline OptionsMenu(Engine * engine) : Menu(engine), m_engine(engine) {
		// Title
		m_title->setText("OPTIONS");

		// Add 'Video' button
		auto videoButton = std::make_shared<Button>(engine, "VIDEO");
		m_videoMenu = std::make_shared<Options_Video>(engine);
		m_videoMenu->setVisible(false);
		addButton(videoButton, [&]() { pressVideo(); });
		addElement(m_videoMenu);

		// Add 'Graphics' button
		auto graphicsButton = std::make_shared<Button>(engine, "GRAPHICS");
		m_gfxMenu = std::make_shared<Options_Graphics>(engine);
		m_gfxMenu->setVisible(false);
		addButton(graphicsButton, [&]() { pressGraphics(); });
		addElement(m_gfxMenu);

		// Add 'Controls' button
		auto controlsButton = std::make_shared<Button>(engine, "CONTROLS");
		controlsButton->setEnabled(false);
		addButton(controlsButton, [&]() { pressControls(); });

		// Add 'Back' button
		auto backButton = std::make_shared<Button>(engine, "< BACK  ");
		addButton(backButton, [&]() { pressBack(); });
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_videoMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_gfxMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_videoMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		m_gfxMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		Menu::setScale(scale);
	}
	inline virtual void userAction(ActionState & actionState) override {
		if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS)
			pressBack();
		else
			m_layout->userAction(actionState);
	}


protected:	
	// Protected Methods
	/***/
	inline void pressVideo() {
		m_videoMenu->setVisible(true);
		m_gfxMenu->setVisible(false);
		// Transfer control only to the video menu
		m_engine->getModule_UI().pushFocusedElement(m_videoMenu);
		enactCallback(on_video);
	}
	/***/
	inline void pressGraphics() {
		m_videoMenu->setVisible(false);
		m_gfxMenu->setVisible(true);
		// Transfer control only to the video menu
		m_engine->getModule_UI().pushFocusedElement(m_gfxMenu);
		enactCallback(on_graphics);
	}
	/***/
	inline void pressControls() {
		enactCallback(on_controls);
	}
	/***/
	inline void pressBack() {
		m_videoMenu->setVisible(false);
		m_gfxMenu->setVisible(false);
		m_engine->getPreferenceState().save();
		// Revert appearance and control back to previous element (start menu, pause menu, etc)
		m_engine->getModule_UI().popRootElement();
		m_layout->setSelectionIndex(-1);
		enactCallback(on_back);
	}


	// Protected Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<UI_Element> m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H