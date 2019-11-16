#pragma once
#ifndef MENU_H
#define MENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Separator.h"
#include "Engine.h"


/** A UI element serving as a menu.
Made to be sub-classed and expanded upon, provides a method for adding menu buttons. */
class Menu : public UI_Element {
public:
	// Public (De)Constructors
	/** Destroy the menu. */
	inline ~Menu() = default;
	/** Construct a menu.
	@param	engine		the engine to use. */
	inline explicit Menu(Engine* engine) noexcept :
		UI_Element(engine),
		m_backPanel(std::make_shared<Panel>(engine)),
		m_layout(std::make_shared<List>(engine)),
		m_title(std::make_shared<Label>(engine)),
		m_separator(std::make_shared<Separator>(engine))
	{
		// Make a background panel for cosmetic purposes
		m_backPanel->setColor(glm::vec4(0.1, 0.1, 0.1, 0.5));
		addElement(m_backPanel);

		// Make a vertical layout to house list items
		m_layout->setSpacing(10.0f);
		m_layout->addCallback((int)List::Interact::on_selection, [&]() {
			const auto index = m_layout->getSelectionIndex();
			if (index >= 0 && index < m_selectionCallbacks.size())
				m_selectionCallbacks[index]();
			});
		m_backPanel->addElement(m_layout);

		// Title
		m_title->setTextScale(15.0f);
		m_title->setAlignment(Label::Alignment::align_center);
		m_title->setColor(glm::vec3(0.8, 0.6, 0.1));
		m_backPanel->addElement(m_title);

		// Title Separator
		m_backPanel->addElement(m_separator);

		// Callbacks
		addCallback((int)UI_Element::Interact::on_resize, [&]() {
			const auto scale = getScale();
			m_backPanel->setScale({ 128, scale.y });
			m_backPanel->setPosition(glm::vec2(256, scale.y));
			m_layout->setScale({ 128, 128 });
			m_layout->setPosition(glm::vec2(0, -500));
			m_title->setScale({ 128, scale.y });
			m_title->setPosition({ 0, -300 });
			m_separator->setScale({ 128, scale.y });
			m_separator->setPosition({ 0, -325 });
			});
	}


	// Public Interface Implementations
	inline virtual void userAction(ActionState& actionState) noexcept override {
		// Start menu doesn't implement any custom controls, focus is on the list
		m_layout->userAction(actionState);
	}


	// Public Methods
	/** Retrieve this menu's focus map.
	@return				this menu's focus map. */
	inline auto getFocusMap() const noexcept {
		return m_focusMap;
	}


protected:
	// Protected Methods
	/** Create a button with the text and callback specified.
	@param	engine		the engine to use.
	@param	buttonText	the text to label the button with.
	@param	callback	the callback to use when the button is pressed. */
	inline void addButton(Engine* engine, const char* buttonText, const std::function<void()>& callback) noexcept {
		auto button = std::make_shared<Button>(engine, buttonText);
		button->setScale({ 120, 20 });
		button->addCallback((int)Button::Interact::on_clicked, callback);
		m_selectionCallbacks.push_back(callback);
		m_layout->addElement(button);
		m_layout->getFocusMap().addElement(button);
	};


	// Protected Attributes
	std::shared_ptr<Panel> m_backPanel;
	std::shared_ptr<List> m_layout;
	std::shared_ptr<Label> m_title;
	std::shared_ptr<Separator> m_separator;
	std::shared_ptr<FocusMap> m_focusMap;
	std::vector<std::function<void()>> m_selectionCallbacks;
};

#endif // MENU_H
