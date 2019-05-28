#pragma once
#ifndef OPTIONS_PANE_H
#define OPTIONS_PANE_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/Layout_Horizontal.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Separator.h"
#include "Engine.h"


/** A UI element serving as an options panel, with a title and a description.
Made to be subclassed and expanded upon. Provides a method for adding new settings. */
class Options_Pane : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy the options pane. */
	inline ~Options_Pane() = default;
	/** Construct a options pane.
	@param	engine		the engine to use. */
	inline Options_Pane(Engine * engine) {
		// Make a background panel for cosemetic purposes
		auto panel = std::make_shared<Panel>(engine);
		panel->setColor(glm::vec4(0.20f));
		m_backPanel = panel;
		addElement(panel);

		// Make a vertical layout to house list items
		auto layout = std::make_shared<List>(engine);
		layout->setSpacing(1.0f);
		layout->setMargin(50.0f);
		layout->addCallback(List::on_selection, [&]() {
			const auto index = (std::dynamic_pointer_cast<List>(m_layout)->getIndex());
			if (index > -1 && size_t(index) < m_descriptions.size())
				std::dynamic_pointer_cast<Label>(m_description)->setText(m_descriptions[index]);
			else
				std::dynamic_pointer_cast<Label>(m_description)->setText("");
		});
		m_layout = layout;
		m_backPanel->addElement(layout);

		// Title
		m_title = std::make_shared<Label>(engine, "");
		m_title->setTextScale(20.0f);
		m_title->setAlignment(Label::align_left);
		m_backPanel->addElement(m_title);

		// Top Separator
		m_separatorTop = std::make_shared<Separator>(engine);
		m_backPanel->addElement(m_separatorTop);

		// Bottom Separator
		m_separatorBot = std::make_shared<Separator>(engine);
		m_backPanel->addElement(m_separatorBot);

		// Bottom Description Label
		auto description = std::make_shared<Label>(engine);
		description->setAlignment(Label::align_left);
		description->setTextScale(10.0f);
		description->setText("");
		m_description = description;
		m_backPanel->addElement(description);
	}


	// Public Interface Implementations
	inline virtual void setScale(const glm::vec2 & scale) override {
		m_backPanel->setScale(scale);
		m_layout->setScale(scale - glm::vec2(0, 50));
		m_title->setPosition({ -scale.x + 50, scale.y - 50 });
		m_separatorTop->setScale(scale);
		m_separatorTop->setPosition({ 0, scale.y - 100 });
		m_separatorBot->setScale(scale);
		m_separatorBot->setPosition({ 0, -scale.y + 100 });
		m_description->setPosition({ -scale.x + 50, -scale.y + 50 });
		UI_Element::setScale(scale);
	}
	

protected:
	// Protected Methods
	inline void addOption(Engine * engine, std::shared_ptr<UI_Element> element, const std::string & text, const std::string & description) {
		auto horizontalLayout = std::make_shared<Layout_Horizontal>();
		auto label = std::make_shared<Label>(engine, text);
		label->setColor(glm::vec3(0.75f));
		horizontalLayout->addElement(label);
		horizontalLayout->addElement(element);
		horizontalLayout->setScale({ 0, 30 });
		m_layout->addElement(horizontalLayout);
		m_descriptions.push_back(description);
	};


	// Protected Attributes
	std::shared_ptr<Label> m_title;
	std::shared_ptr<UI_Element> m_backPanel, m_layout, m_separatorTop, m_separatorBot, m_description;
	std::vector<std::string> m_descriptions;
};

#endif // OPTIONS_PANE_H