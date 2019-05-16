#pragma once
#ifndef MENU_H
#define MENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Separator.h"
#include "Engine.h"


/** A UI element serving as a menu. */
class Menu : public UI_Element
{
public:
	// (de)Constructors
	inline ~Menu() = default;
	inline Menu(Engine * engine) : UI_Element() {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.20f));
		m_backPanel = panel;
		addElement(panel);

		// Make a vertical layout to house list items
		auto layout = std::make_shared<List>(engine);
		layout->setSpacing(10.0f);
		m_layout = layout;
		m_backPanel->addElement(layout);

		// Title
		m_title = std::make_shared<Label>(engine, "");
		m_title->setTextScale(15.0f);
		m_title->setAlignment(Label::align_center);
		m_backPanel->addElement(m_title);

		// Title Separator
		m_separator = std::make_shared<Separator>(engine);
		m_backPanel->addElement(m_separator);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_backPanel->setScale({ 128, scale.y });
		m_backPanel->setPosition(glm::vec2(256, scale.y));
		m_layout->setScale({ 128, 128 });
		m_layout->setPosition(glm::vec2(0, -500));
		m_title->setPosition({ 0, -300 });
		m_separator->setScale({ 128, scale.y });
		m_separator->setPosition({ 0, -325 });
		UI_Element::setScale(scale);
	}


protected:
	// Protected Methods
	void addButton(std::shared_ptr<UI_Element> element) {
		element->setScale({ 120, 20 });
		m_layout->addElement(element);
	};


	// Private Attributes
	std::shared_ptr<Label> m_title;
	std::shared_ptr<UI_Element> m_backPanel, m_separator, m_layout;
};

#endif // STARTMENU_H