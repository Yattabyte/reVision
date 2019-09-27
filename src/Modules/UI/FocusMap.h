#pragma once
#ifndef FOCUSMAP_H
#define FOCUSMAP_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Utilities/ActionState.h"
#include <map>
#include <vector>


/** This class contains a list of UI elements, where one specific one is focused on.
Focus can be changed manually, or by user input directly.
Goal is to decouple basic UI elements from changing focus, and have more complex ones use FocusMap to standardize movement between them. */
class FocusMap {
public:
	// Public (de)Constructors
	/** Destroy this focus map. */
	inline ~FocusMap() = default;
	/** Construct a focus map. */
	inline FocusMap() = default;


	// Public Methods
	/** Add an element to this focus map, setting it in focus.
	@param	element			the new element to add to the map. */
	inline void addElement(const std::shared_ptr<UI_Element>& element) {
		m_elements.push_back(element);
	}
	/** Remove an element from this focus map, comparing its underlying pointer.
	@param	element			the element to remove from the map.
	@return					true if found and removed, false otherwise. */
	inline bool removeElement(const std::shared_ptr<UI_Element>& element) {
		size_t index(0ull);
		bool found = false;
		for each (const auto & e in m_elements) {
			if (e.get() == element.get()) {
				found = true;
				break;
			}
			index++;
		}

		if (found)
			m_elements.erase(m_elements.begin() + index);
		return found;
	}
	/** Remove all elements from this focus map. */
	inline void clear() {
		m_elements.clear();
		m_index = -1;
	}
	/** Set the focus onto a specific index, if it is able to be focused on.
	@param	newIndex		the index to attempt to focus on. */
	inline void focusIndex(const int& newIndex) {
		if (newIndex >= 0 && newIndex < m_elements.size() && elementFocusable(m_elements[newIndex]))
			m_index = newIndex;
	}
	/** Set the focus onto a specific element, if it is able to be focused on, if it's pointer can be found.
	@param	element			the element to attempt to focus on.
	@return					true if found and focused, false otherwise. */
	inline bool focusElement(const std::shared_ptr<UI_Element>& element) {
		int index(0);
		bool found = false;
		for each (const auto & e in m_elements) {
			if (e.get() == element.get()) {
				found = true;
				break;
			}
			index++;
		}

		if (found)
			focusIndex(index);
		return found;
	}
	/** Apply user input to the focus map, forwarding it to the focused element and perhaps changing the focus as well.
	@param	actionState		the action state to apply. */
	inline void applyActionState(ActionState& actionState) {
		if (m_elements.size() && m_index >= 0) {
			if (elementFocusable(m_elements[m_index]))
				m_elements[m_index]->userAction(actionState);

			// Switch focus last, let element try to consume input first
			if (actionState.isAction(ActionState::UI_LEFT) == ActionState::PRESS)
				back();
			else if (actionState.isAction(ActionState::UI_RIGHT) == ActionState::PRESS)
				forward();
		}
	}
	/** Move the focus back by 1. */
	inline void back() {
		focusIndex(m_index - 1);
	}
	/** Move the focus ahead by 1. */
	inline void forward() {
		focusIndex(m_index + 1);
	}


private:
	// Private Methods
	/** Shorthand method returning if we can tab/switch to an element.
	@param	element		the element to check.
	@return				true if focusable, false otherwise. */
	inline bool elementFocusable(const std::shared_ptr<UI_Element>& element) const {
		return element->getVisible() && element->getEnabled();
	};



	// Private Attributes
	int m_index = -1;
	std::vector<std::shared_ptr<UI_Element>> m_elements;
};

#endif // FOCUSMAP_H