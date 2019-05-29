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
		videoButton->addCallback(Button::on_clicked, [&]() { pressVideo(); });
		addButton(videoButton);
		addElement(m_videoMenu);

		// Add 'Graphics' button
		auto graphicsButton = std::make_shared<Button>(engine, "GRAPHICS");
		m_gfxMenu = std::make_shared<Options_Graphics>(engine);
		m_gfxMenu->setVisible(false);
		graphicsButton->addCallback(Button::on_clicked, [&]() { pressGraphics(); });
		addButton(graphicsButton);
		addElement(m_gfxMenu);

		// Add 'Controls' button
		auto controlsButton = std::make_shared<Button>(engine, "CONTROLS");
		controlsButton->setEnabled(false);
		controlsButton->addCallback(Button::on_clicked, [&]() {	pressControls(); });
		addButton(controlsButton);

		// Add 'Back' button
		auto backButton = std::make_shared<Button>(engine, "< BACK  ");
		backButton->addCallback(Button::on_clicked, [&]() { pressBack(); });
		addButton(backButton);
		m_layout->setIndex(0);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_videoMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_gfxMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f });
		m_videoMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		m_gfxMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		Menu::setScale(scale);
	}
	inline virtual bool userAction(ActionState & actionState) override {
		if (actionState.isAction(ActionState::UI_UP) == ActionState::PRESS) {
			m_layout->setIndex(m_layout->getIndex() - 1);
			return true;
		}
		else if (actionState.isAction(ActionState::UI_DOWN) == ActionState::PRESS) {
			m_layout->setIndex(m_layout->getIndex() + 1);
			return true;
		}
		else if (actionState.isAction(ActionState::UI_ENTER) == ActionState::PRESS) {
			if (auto index = m_layout->getIndex(); index > -1)
				m_layout->getElement(index)->fullPress();
			return true;
		}
		else if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS) {
			pressBack();
			return true;
		}
		return false;
	}


protected:	
	// Protected Methods
	/***/
	inline void pressVideo() {
		m_videoMenu->setVisible(true);
		m_gfxMenu->setVisible(false);
		m_engine->getModule_UI().pushFocusedElement(m_videoMenu);
		enactCallback(on_video);
	}
	/***/
	inline void pressGraphics() {
		m_videoMenu->setVisible(false);
		m_gfxMenu->setVisible(true);
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
		m_engine->getModule_UI().popRootElement();
		enactCallback(on_back);
	}


	// Protected Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<UI_Element> m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H