#pragma once
#ifndef MENU_H
#define MENU_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/Basic Elements/Label.h"
#include "Modules/UI/Basic Elements/List.h"
#include "Modules/UI/Basic Elements/Panel.h"
#include "Modules/UI/Basic Elements/Separator.h"


/** A UI element serving as a menu.
Made to be sub-classed and expanded upon, provides a method for adding menu buttons. */
class Menu : public UI_Element {
public:
	// Public (De)Constructors
	/** Construct a menu.
	@param	engine		reference to the engine to use. */
	explicit Menu(Engine& engine);


	// Public Interface Implementations
	void userAction(ActionState& actionState) override;


	// Public Methods
	/** Retrieve this menu's focus map.
	@return				this menu's focus map. */
	std::shared_ptr<FocusMap> getFocusMap() const noexcept;


protected:
	// Protected Methods
	/** Create a button with the text and callback specified.
	@param	engine		reference to the engine to use. 
	@param	buttonText	the text to label the button with.
	@param	callback	the callback to use when the button is pressed. */
	void addButton(Engine& engine, const char* buttonText, const std::function<void()>& callback);


	// Protected Attributes
	std::shared_ptr<Panel> m_backPanel;
	std::shared_ptr<List> m_layout;
	std::shared_ptr<Label> m_title;
	std::shared_ptr<Separator> m_separator;
	std::shared_ptr<FocusMap> m_focusMap;
	std::vector<std::function<void()>> m_selectionCallbacks;
};

#endif // MENU_H