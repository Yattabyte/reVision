#pragma once
#ifndef FOCUSMAP_H
#define FOCUSMAP_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Utilities/ActionState.h"
#include <vector>


/** This class contains a list of UI elements, where one specific one is focused on.
Focus can be changed manually, or by user input directly.
Goal is to decouple basic UI elements from changing focus, and have more complex ones use FocusMap to standardize movement between them. */
class FocusMap {
public:
	// Public (De)Constructors
	/** Destroy this focus map. */
	inline ~FocusMap() noexcept = default;
	/** Construct a focus map. */
	inline FocusMap() noexcept = default;


	// Public Methods
	/** Add an element to this focus map, setting it in focus.
	@param	element			the new element to add to the map. */
	void addElement(const std::shared_ptr<UI_Element>& element) noexcept;
	/** Remove an element from this focus map, comparing its underlying pointer.
	@param	element			the element to remove from the map.
	@return					true if found and removed, false otherwise. */
	bool removeElement(const std::shared_ptr<UI_Element>& element) noexcept;
	/** Remove all elements from this focus map. */
	void clear() noexcept;
	/** Set the focus onto a specific index, if it is able to be focused on.
	@param	newIndex		the index to attempt to focus on. */
	void focusIndex(const int& newIndex) noexcept;
	/** Set the focus onto a specific element, if it is able to be focused on, if it's pointer can be found.
	@param	element			the element to attempt to focus on.
	@return					true if found and focused, false otherwise. */
	bool focusElement(const std::shared_ptr<UI_Element>& element) noexcept;
	/** Apply user input to the focus map, forwarding it to the focused element and perhaps changing the focus as well.
	@param	actionState		the action state to apply. */
	void applyActionState(ActionState& actionState) noexcept;
	/** Move the focus back by 1. */
	void back() noexcept;
	/** Move the focus ahead by 1. */
	void forward() noexcept;


private:
	// Private Methods
	/** Shorthand method returning if we can tab/switch to an element.
	@param	element		the element to check.
	@return				true if focusable, false otherwise. */
	bool static elementFocusable(const std::shared_ptr<UI_Element>& element) noexcept;


	// Private Attributes
	int m_index = -1;
	std::vector<std::shared_ptr<UI_Element>> m_elements;
};

#endif // FOCUSMAP_H