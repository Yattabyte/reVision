#pragma once
#ifndef OPTIONS_PANE_H
#define OPTIONS_PANE_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Separator.h"


/** A UI element serving as an options panel, with a title and a description.
Made to be sub-classed and expanded upon. Provides a method for adding new settings. */
class Options_Pane : public UI_Element {
public:
	// Public (De)Constructors
	/** Construct a options pane.
	@param	engine		reference to the engine to use. */
	explicit Options_Pane(Engine& engine);


	// Public Interface Implementations
	void userAction(ActionState& actionState) override;


protected:
	// Protected Methods
	/** Add an option to the options menu.
	@param	engine		reference to the engine to use. 
	@param	element		the element to add to the options menu.
	@param	text		the text to title the option.
	@param	description	the text to describe the option. */
	void addOption(Engine& engine, const std::shared_ptr<UI_Element>& element, const float& ratio, const std::string& text, const std::string& description, const int& eventType, const std::function<void()>& callback);


	// Protected Attributes
	std::shared_ptr<Label> m_title, m_description;
	std::shared_ptr<List> m_layout;
	std::shared_ptr<Separator> m_separatorTop, m_separatorBot;
	std::shared_ptr<Panel> m_backPanel;
	std::vector<std::string> m_descriptions;
	std::vector<std::shared_ptr<UI_Element>> m_elements;
};

#endif // OPTIONS_PANE_H