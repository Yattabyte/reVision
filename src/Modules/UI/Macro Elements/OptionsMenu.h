#pragma once
#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Layout_Vertical.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Macro Elements/Options_Video.h"
#include "Modules/UI/Macro Elements/Options_Graphics.h"
#include "Engine.h"


/** A UI element serving as an options menu. */
class OptionsMenu : public UI_Element
{
public:
	// UI interaction enums
	enum interact {
		on_video = last_interact_index,
		on_graphics,
		on_controls,
		on_back,
	};

	// (de)Constructors
	inline ~OptionsMenu() = default;
	inline OptionsMenu(Engine * engine) : UI_Element() {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor({ 0.1, 0.1, 0.1 });
		m_backPanel = panel;
		addElement(panel);

		// Make a vertical layout to house list items
		auto layout = std::make_shared<Layout_Vertical>(engine);
		layout->setSpacing(10.0f);
		m_layout = layout;
		m_backPanel->addElement(layout);
		
		// Title
		auto title = std::make_shared<Label>(engine, "Options Menu");
		title->setTextScale(20.0f);
		title->setAlignment(Label::align_center);
		m_title = title;
		m_backPanel->addElement(title);

		// Add 'Video' button
		auto videoButton = std::make_shared<Button>(engine, "Video");
		m_videoMenu = std::make_shared<Options_Video>(engine);
		m_videoMenu->setVisible(true);
		videoButton->addCallback(UI_Element::on_mouse_release, [&]() { 
			m_videoMenu->setVisible(true);
			m_gfxMenu->setVisible(false);
			enactCallback(on_video); 
		});
		layout->addElement(videoButton);
		addElement(m_videoMenu);

		// Add 'Graphics' button
		auto graphicsButton = std::make_shared<Button>(engine, "Graphics");
		m_gfxMenu = std::make_shared<Options_Graphics>(engine);
		m_gfxMenu->setVisible(false);
		graphicsButton->addCallback(UI_Element::on_mouse_release, [&]() {
			m_videoMenu->setVisible(false);
			m_gfxMenu->setVisible(true);
			enactCallback(on_graphics); 
		});
		layout->addElement(graphicsButton);
		addElement(m_gfxMenu);

		// Add 'Controls' button
		auto controlsButton = std::make_shared<Button>(engine, "Controls (disabled)");
		controlsButton->setEnabled(false);
		controlsButton->addCallback(UI_Element::on_mouse_release, [&]() { 
			enactCallback(on_controls); 
		});
		layout->addElement(controlsButton);

		// Add 'Back' button
		auto backButton = std::make_shared<Button>(engine, "<       Back        ");
		backButton->addCallback(UI_Element::on_mouse_release, [&, engine]() {
			setVisible(false);
			m_videoMenu->setVisible(false);
			m_gfxMenu->setVisible(false);
			engine->getPreferenceState().save();
			enactCallback(on_back); 
		});
		layout->addElement(backButton);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		UI_Element::setScale(scale);
		m_backPanel->setScale({ 125, scale.y });
		m_backPanel->setPosition(glm::vec2(250, scale.y));
		m_layout->setScale({ 125, 125 });
		m_layout->setPosition(glm::vec2(0, -425));
		m_title->setPosition({ 0, -200 });
		m_videoMenu->setScale({ (scale.x / 2.0f) - 250.0f, scale.y / 2.0f});
		m_gfxMenu->setScale({ (scale.x / 2.0f) - 250.0f, scale.y / 2.0f});
		m_videoMenu->setPosition({ (scale.x / 2.0f) + 250.0f, scale.y / 2.0f });
		m_gfxMenu->setPosition({ (scale.x / 2.0f) + 250.0f, scale.y / 2.0f });
		enactCallback(on_resize);
	}


private:
	// Private Attributes
	std::shared_ptr<UI_Element> m_backPanel, m_title, m_layout, m_videoMenu, m_gfxMenu;
};

#endif // OPTIONSMENU_H