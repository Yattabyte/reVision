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
	enum interact {
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
	inline OptionsMenu(Engine * engine) : Menu(engine) {
		// Title
		m_title->setText("OPTIONS");

		// Add 'Video' button
		auto videoButton = std::make_shared<Button>(engine, "VIDEO");
		m_videoMenu = std::make_shared<Options_Video>(engine);
		m_videoMenu->setVisible(false);
		videoButton->addCallback(Button::on_clicked, [&]() {
			m_videoMenu->setVisible(true);
			m_gfxMenu->setVisible(false);
			enactCallback(on_video); 
		});
		addButton(videoButton);
		addElement(m_videoMenu);

		// Add 'Graphics' button
		auto graphicsButton = std::make_shared<Button>(engine, "GRAPHICS");
		m_gfxMenu = std::make_shared<Options_Graphics>(engine);
		m_gfxMenu->setVisible(false);
		graphicsButton->addCallback(Button::on_clicked, [&]() {
			m_videoMenu->setVisible(false);
			m_gfxMenu->setVisible(true);
			enactCallback(on_graphics); 
		});
		addButton(graphicsButton);
		addElement(m_gfxMenu);

		// Add 'Controls' button
		auto controlsButton = std::make_shared<Button>(engine, "CONTROLS");
		controlsButton->setEnabled(false);
		controlsButton->addCallback(Button::on_clicked, [&]() {
			enactCallback(on_controls); 
		});
		addButton(controlsButton);

		// Add 'Back' button
		auto backButton = std::make_shared<Button>(engine, "< BACK  ");
		backButton->addCallback(Button::on_clicked, [&, engine]() {
			setVisible(false);
			m_videoMenu->setVisible(false);
			m_gfxMenu->setVisible(false);
			engine->getPreferenceState().save();
			enactCallback(on_back); 
		});
		addButton(backButton);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_videoMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f});
		m_gfxMenu->setScale({ (scale.x / 2.0f) - 320.0f, scale.y / 2.0f});
		m_videoMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		m_gfxMenu->setPosition({ (scale.x / 2.0f) + 192.0f, scale.y / 2.0f });
		Menu::setScale(scale);
	}


protected:
	// Protected Attributes
	std::shared_ptr<UI_Element> m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H