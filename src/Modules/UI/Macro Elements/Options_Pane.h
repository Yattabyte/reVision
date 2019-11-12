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
	inline explicit Options_Pane(Engine* engine) :
		UI_Element(engine),
		m_title(std::make_shared<Label>(engine)),
		m_description(std::make_shared<Label>(engine)),
		m_layout(std::make_shared<List>(engine)),
		m_separatorTop(std::make_shared<Separator>(engine)),
		m_separatorBot(std::make_shared<Separator>(engine)),
		m_backPanel(std::make_shared<Panel>(engine))
	{
		// Make a background panel for cosemetic purposes
		m_backPanel->setColor(glm::vec4(0.1, 0.1, 0.1, 0.5));
		addElement(m_backPanel);

		// Make a vertical layout to house list items
		m_layout->setSpacing(1.0f);
		m_layout->setMargin(50.0f);
		m_layout->addCallback(List::on_selection, [&]() {
			const auto index = m_layout->getSelectionIndex();
			if (index > -1 && size_t(index) < m_descriptions.size())
				std::dynamic_pointer_cast<Label>(m_description)->setText(m_descriptions[index]);
			else
				std::dynamic_pointer_cast<Label>(m_description)->setText("");
			});
		m_backPanel->addElement(m_layout);

		// Title
		m_title->setTextScale(20.0f);
		m_title->setAlignment(Label::align_left);
		m_backPanel->addElement(m_title);

		// Top Separator
		m_backPanel->addElement(m_separatorTop);

		// Bottom Separator
		m_backPanel->addElement(m_separatorBot);

		// Bottom Description Label
		m_description->setAlignment(Label::align_left);
		m_description->setTextScale(10.0f);
		m_description->setColor(glm::vec3(0.8, 0.6, 0.1));
		m_description->setText("");
		m_backPanel->addElement(m_description);

		// Callbacks
		addCallback(UI_Element::on_resize, [&]() {
			const auto scale = getScale();
			m_backPanel->setScale(scale);
			m_layout->setScale(scale - glm::vec2(0, 50));
			m_title->setScale(scale);
			m_title->setPosition({ 50, scale.y - 50 });
			m_separatorTop->setScale(scale);
			m_separatorTop->setPosition({ 0, scale.y - 100 });
			m_separatorBot->setScale(scale);
			m_separatorBot->setPosition({ 0, -scale.y + 100 });
			m_description->setScale(scale);
			m_description->setPosition({ 50, -scale.y + 50 });
			});
	}


	// Public Interface Implementations
	inline virtual void userAction(ActionState& actionState) override {
		// Options menu doesn't implement any custom controls, focus is on the list
		m_layout->userAction(actionState);
		if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS)
			m_engine->getModule_UI().getFocusMap()->back();
	}


protected:
	// Protected Methods
	/** Add an option to the options menu.
	@param	engine		the engine to use.
	@param	element		the element to add to the options menu.
	@param	text		the text to title the option.
	@param	description	the text to describe the option. */
	inline void addOption(Engine* engine, std::shared_ptr<UI_Element> element, const float& ratio, const std::string& text, const std::string& description, const int& eventType, const std::function<void()>& callback) {
		auto horizontalLayout = std::make_shared<Layout_Horizontal>(engine);
		auto label = std::make_shared<Label>(engine, text);
		label->setColor(glm::vec3(0.75f));
		element->addCallback(eventType, callback);
		element->setMaxHeight(14.0f);
		horizontalLayout->addElement(label, (1.0f - ratio) + 1.0f);
		horizontalLayout->addElement(element, ratio);
		horizontalLayout->setScale({ 0, 30 });
		m_layout->addElement(horizontalLayout);
		m_layout->getFocusMap().addElement(element);
		m_elements.push_back(element);
		m_descriptions.push_back(description);
	};


	// Protected Attributes
	std::shared_ptr<Label> m_title, m_description;
	std::shared_ptr<List> m_layout;
	std::shared_ptr<Separator> m_separatorTop, m_separatorBot;
	std::shared_ptr<Panel> m_backPanel;
	std::vector<std::string> m_descriptions;
	std::vector<std::shared_ptr<UI_Element>> m_elements;
};

#endif // OPTIONS_PANE_H